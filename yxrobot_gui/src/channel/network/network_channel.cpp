#include "network_channel.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <thread>
#include <vector>

#include "base_proto/geometry.pb.h"
#include "base_proto/map.pb.h"
#include "sensor_proto/smart_sensors.pb.h"

namespace {

uint16_t readU16(const uint8_t* data)
{
    return static_cast<uint16_t>(data[0]) |
           (static_cast<uint16_t>(data[1]) << 8);
}

uint32_t readU32(const uint8_t* data)
{
    uint32_t value = 0;
    for (int i = 0; i < 4; ++i) {
        value |= static_cast<uint32_t>(data[i]) << (i * 8);
    }
    return value;
}

uint64_t readU64(const uint8_t* data)
{
    uint64_t value = 0;
    for (int i = 0; i < 8; ++i) {
        value |= static_cast<uint64_t>(data[i]) << (i * 8);
    }
    return value;
}

uint16_t readPortFromEnv()
{
    const char* value = std::getenv("YX_TELEMETRY_PORT");
    if (!value) {
        return 11086;
    }

    const long port = std::strtol(value, nullptr, 10);
    if (port <= 0 || port > std::numeric_limits<uint16_t>::max()) {
        return 11086;
    }
    return static_cast<uint16_t>(port);
}

int toOccupancyValue(uint8_t cell)
{
    switch (cell) {
    case silverstar::map::FREE:
        return 0;
    case silverstar::map::OCCUPIED:
        return 100;
    case silverstar::map::UNKNOWN:
        return -1;
    default:
        return static_cast<int>(cell);
    }
}

inline float normalizeRadianAngle(const float angle) {
  // float a = std::fmod(angle + M_PI, 2.0 * M_PI);
  // if (a < 0.0) {
  //   a += (2.0 * M_PI);
  // }
  // return a - M_PI;
  if (angle > M_PI) {
    return angle - 2.0 * M_PI;
  }
  if (angle < -M_PI) {
    return angle + 2.0 * M_PI;
  }
  return angle;
}

// 1. 地图转换函数：处理Y轴倒序，将 proto 数据安全写入到 OccupancyMap 中
inline void ConvertSilverStarMapDataToROS(
    const silverstar::map::OccupancyGridProto& proto,
    OccupancyMap& map) 
{
    const int width = static_cast<int>(proto.info().width());
    const int height = static_cast<int>(proto.info().height());
    const std::string& cells = proto.data();

    for (int row = 0; row < height; ++row) {
        // ROS 地图 Y 轴倒序
        const int ros_row = height - 1 - row; 
        for (int col = 0; col < width; ++col) {
            const auto cell = static_cast<uint8_t>(cells[static_cast<size_t>(row * width + col)]);
            // 写入反转后的行
            map(ros_row, col) = toOccupancyValue(cell); 
        }
    }
}

// 2. 位姿转换函数：SilverStar -> ROS
inline void TransformPoseToROS(
    const float in_x, const float in_y, const float in_yaw,
    const float map_width, const float ox, const float oy,
    double& out_x, double& out_y, double& out_yaw) 
{
    out_x = map_width - (in_y - oy) + ox;
    out_y = in_x - ox + oy;
    // +90度 偏置
    out_yaw = normalizeRadianAngle(in_yaw + M_PI_2); 
}

// 3. 局部激光点转换函数：处理无全局位姿时的 +90度 局部坐标系旋转
inline void TransformLocalPointToROS(
    const double in_lx, const double in_ly, 
    double& out_x, double& out_y) 
{
    // 旋转矩阵推导：+90度旋转 -> x' = -y, y' = x
    out_x = -in_ly;
    out_y = in_lx;
}

}  // namespace

NetworkChannel::NetworkChannel()
{
    const char* host = std::getenv("YX_TELEMETRY_HOST");
    host_ = host ? host : "127.0.0.1";
    port_ = readPortFromEnv();
}

NetworkChannel::~NetworkChannel()
{
    ShutDown();
}

bool NetworkChannel::Start()
{
    return true;
}

void NetworkChannel::Process()
{
    if (!connected_ && !connectToServer()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        return;
    }

    telemetry_protocol::FrameHeader header;
    std::string payload;
    if (!receiveFrame(header, payload)) {
        disconnectFromServer();
        return;
    }

    handleFrame(header, payload);
}

bool NetworkChannel::Stop()
{
    disconnectFromServer();
    return true;
}

bool NetworkChannel::connectToServer()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return false;
    }

    int flags = fcntl(fd, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port_);
    if (inet_pton(AF_INET, host_.c_str(), &address.sin_addr) != 1) {
        close(fd);
        return false;
    }

    int ret = ::connect(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address));
    if (ret < 0 && errno != EINPROGRESS) {
        close(fd);
        return false;
    }

    pollfd pfd{};
    pfd.fd = fd;
    pfd.events = POLLOUT;
    if (poll(&pfd, 1, 300) <= 0) {
        close(fd);
        return false;
    }

    int error = 0;
    socklen_t error_len = sizeof(error);
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &error_len) < 0 || error != 0) {
        close(fd);
        return false;
    }

    if (flags >= 0) {
        fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    }

    timeval timeout{};
    timeout.tv_sec = 0;
    timeout.tv_usec = 50000;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    socket_fd_ = fd;
    connected_ = true;
    std::cout << "connected telemetry server " << host_ << ":" << port_ << std::endl;
    return true;
}

void NetworkChannel::disconnectFromServer()
{
    if (socket_fd_ >= 0) {
        close(socket_fd_);
        socket_fd_ = -1;
    }
    connected_ = false;
}

bool NetworkChannel::receiveExact(void* data, size_t size)
{
    auto* out = static_cast<uint8_t*>(data);
    size_t received = 0;
    while (received < size) {
        const ssize_t ret = recv(socket_fd_, out + received, size - received, 0);
        if (ret > 0) {
            received += static_cast<size_t>(ret);
            continue;
        }
        if (ret < 0 && errno == EINTR) {
            continue;
        }
        return false;
    }
    return true;
}

bool NetworkChannel::receiveFrame(telemetry_protocol::FrameHeader& header, std::string& payload)
{
    uint8_t header_data[telemetry_protocol::kFrameHeaderSize] = {0};
    if (!receiveExact(header_data, sizeof(header_data))) {
        return false;
    }

    header.magic = readU32(header_data);
    header.version = readU16(header_data + 4);
    header.topic = readU16(header_data + 6);
    header.sequence = readU64(header_data + 8);
    header.payload_size = readU32(header_data + 16);
    header.reserved = readU32(header_data + 20);

    if (header.magic != telemetry_protocol::kFrameMagic ||
        header.version != telemetry_protocol::kFrameVersion) {
        return false;
    }

    payload.resize(header.payload_size);
    if (payload.empty()) {
        return true;
    }
    return receiveExact(payload.data(), payload.size());
}

void NetworkChannel::handleFrame(const telemetry_protocol::FrameHeader& header, const std::string& payload)
{
    switch (static_cast<telemetry_protocol::FrameTopic>(header.topic)) {
    case telemetry_protocol::FrameTopic::Map:
        handleMapPayload(payload);
        break;
    case telemetry_protocol::FrameTopic::LaserScan:
        handleLaserPayload(payload);
        break;
    case telemetry_protocol::FrameTopic::RobotPose:
        handlePosePayload(payload);
        break;
    default:
        break;
    }
}

void NetworkChannel::handleMapPayload(const std::string& payload)
{
    silverstar::map::OccupancyGridProto proto;
    if (!proto.ParseFromString(payload)) {
        return;
    }

    const int width = static_cast<int>(proto.info().width());
    const int height = static_cast<int>(proto.info().height());
    if (width <= 0 || height <= 0 || proto.data().size() < static_cast<size_t>(width * height)) {
        return;
    }

    const float resolution = proto.info().resolution();
    const float ox = proto.info().origin().x();
    const float oy = proto.info().origin().y();

    // 更新缓存信息供位姿转换使用
    map_width_ = width * resolution;
    map_ox_ = ox;
    map_oy_ = oy;
    has_map_info_ = true;

    OccupancyMap map(height, width, resolution, Eigen::Vector3d(ox, oy, 0.0));
    
    // 调用转换函数
    ConvertSilverStarMapDataToROS(proto, map);

    emit emitUpdateMap(map);
}

void NetworkChannel::handleLaserPayload(const std::string& payload)
{
    silverstar::smart_sensor::LaserScanProto proto;
    if (!proto.ParseFromString(payload)) {
        return;
    }

    LaserScan scan;
    scan.id = 1;
    scan.header.frame_id = has_pose_ ? "map" : proto.frame_id();
    scan.header.stamp_ns = proto.stamp().sec() * 1000000000LL + proto.stamp().nsec();
    scan.reserve(static_cast<uint32_t>(proto.data_size()));

    const double c = std::cos(latest_pose_.theta);
    const double s = std::sin(latest_pose_.theta);
    
    for (const auto& sample : proto.data()) {
        const double range = sample.range();
        if (!std::isfinite(range) || range <= 0.0) {
            continue;
        }

        const double angle = sample.angle().rad();
        const double lx = range * std::cos(angle);
        const double ly = range * std::sin(angle);

        Point point;
        if (has_pose_) {
            // 全局位姿已经经过 ROS 转换，直接应用基础的 2D 齐次变换矩阵即可
            point.x = latest_pose_.x + c * lx - s * ly;
            point.y = latest_pose_.y + s * lx + c * ly;
        } else {
            // 无全局位姿时，应用局部坐标系的转换函数
            TransformLocalPointToROS(lx, ly, point.x, point.y);
        }
        scan.push_back(point);
    }

    emit emitUpdateLaserScan(scan);
}

void NetworkChannel::handlePosePayload(const std::string& payload)
{
    silverstar::geometry::PoseStampedProto proto;
    if (!proto.ParseFromString(payload) || !proto.has_pose()) {
        return;
    }

    const float in_x = proto.pose().position().x();
    const float in_y = proto.pose().position().y();
    const float in_yaw = proto.pose().orientation().rad();

    if (has_map_info_) {
        // 调用转换函数：完成 XY 平移映射和偏航角旋转
        TransformPoseToROS(in_x, in_y, in_yaw, map_width_, map_ox_, map_oy_,
                           latest_pose_.x, latest_pose_.y, latest_pose_.theta);
    } else {
        // 容错处理：地图未就绪时，只做偏航角转换，坐标原样透传
        latest_pose_.x = in_x;
        latest_pose_.y = in_y;
        latest_pose_.theta = normalizeRadianAngle(in_yaw + M_PI_2);
    }
    
    has_pose_ = true;

    emit emitUpdateRobotPose(latest_pose_);
}
