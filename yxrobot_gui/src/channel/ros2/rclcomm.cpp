#include "rclcomm.h"

#include <cmath>
#include <functional>

namespace
{
/// 判断导航区域是否能用 Marker 表达。
bool isMarkerZone(const NavigationZone& zone)
{
    if (!zone.enabled) {
        return false;
    }

    if (zone.kind == NavigationZoneKind::VirtualWall) {
        return zone.points.size() >= 2;
    }

    return zone.points.size() >= 3;
}

/// 返回区域类型对应的 Marker 命名空间。
std::string markerNamespace(NavigationZoneKind kind)
{
    switch (kind) {
    case NavigationZoneKind::NoEntry:
        return "no_entry";
    case NavigationZoneKind::VirtualWall:
        return "virtual_wall";
    case NavigationZoneKind::NoMop:
        return "no_mop";
    case NavigationZoneKind::NoSweep:
        return "no_sweep";
    case NavigationZoneKind::Obstacle:
        return "obstacle";
    case NavigationZoneKind::Furniture:
        return "furniture";
    case NavigationZoneKind::CleanArea:
        return "clean_area";
    }

    return "zone";
}

/// 返回区域类型对应的 Marker 颜色。
std_msgs::msg::ColorRGBA markerColor(NavigationZoneKind kind)
{
    std_msgs::msg::ColorRGBA color;
    color.a = 0.95F;
    switch (kind) {
    case NavigationZoneKind::NoEntry:
        color.r = 0.90F;
        color.g = 0.12F;
        color.b = 0.12F;
        break;
    case NavigationZoneKind::VirtualWall:
        color.r = 0.95F;
        color.g = 0.25F;
        color.b = 0.25F;
        break;
    case NavigationZoneKind::NoMop:
        color.r = 0.10F;
        color.g = 0.45F;
        color.b = 1.00F;
        break;
    case NavigationZoneKind::NoSweep:
        color.r = 0.95F;
        color.g = 0.60F;
        color.b = 0.10F;
        break;
    case NavigationZoneKind::Obstacle:
        color.r = 0.45F;
        color.g = 0.25F;
        color.b = 0.12F;
        break;
    case NavigationZoneKind::Furniture:
        color.r = 0.20F;
        color.g = 0.65F;
        color.b = 0.25F;
        break;
    case NavigationZoneKind::CleanArea:
        color.r = 0.10F;
        color.g = 0.85F;
        color.b = 0.45F;
        break;
    }
    return color;
}

/// 将二维导航点追加到 Marker。
void appendMarkerPoint(visualization_msgs::msg::Marker& marker, const NavigationPoint2D& point)
{
    geometry_msgs::msg::Point rosPoint;
    rosPoint.x = point.x;
    rosPoint.y = point.y;
    rosPoint.z = 0.0;
    marker.points.push_back(rosPoint);
}

/// 将通用导航区域转换为 ROS2 Marker 消息。
visualization_msgs::msg::Marker toZoneMarker(
    const NavigationZone& zone,
    const rclcpp::Time& stamp)
{
    visualization_msgs::msg::Marker marker;
    marker.header.stamp = stamp;
    marker.header.frame_id = "map";
    marker.ns = markerNamespace(zone.kind);
    marker.id = static_cast<int>(std::hash<std::string>{}(zone.id) & 0x7FFFFFFF);
    marker.type = visualization_msgs::msg::Marker::LINE_STRIP;
    marker.action = visualization_msgs::msg::Marker::ADD;
    marker.pose.orientation.w = 1.0;
    marker.scale.x = zone.kind == NavigationZoneKind::VirtualWall ? 0.05 : 0.035;
    marker.color = markerColor(zone.kind);
    marker.points.reserve(zone.points.size() + 1);
    for (const NavigationPoint2D& point : zone.points) {
        appendMarkerPoint(marker, point);
    }
    if (zone.kind != NavigationZoneKind::VirtualWall && !zone.points.empty()) {
        appendMarkerPoint(marker, zone.points.front());
    }
    return marker;
}

/// 发布 DELETEALL，用当前区域集合重建 RViz marker 状态。
void publishDeleteAllMarkers(
    const rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr& publisher,
    const rclcpp::Time& stamp)
{
    visualization_msgs::msg::Marker marker;
    marker.header.stamp = stamp;
    marker.header.frame_id = "map";
    marker.action = visualization_msgs::msg::Marker::DELETEALL;
    publisher->publish(marker);
}

/// 将区域集合逐个发布为 Marker。
bool publishZoneMarkers(
    const NavigationZoneCollection& zones,
    const rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr& publisher,
    const rclcpp::Time& stamp)
{
    if (!publisher) {
        return false;
    }

    publishDeleteAllMarkers(publisher, stamp);

    bool published = false;
    for (const NavigationZone& zone : zones.zones) {
        if (!isMarkerZone(zone)) {
            continue;
        }

        publisher->publish(toZoneMarker(zone, stamp));
        published = true;
    }

    return published || zones.zones.empty();
}
}

/// 构造 ROS2 通道对象，实际初始化在 Start 中完成。
rclcomm::rclcomm() {
    
}

/// 析构时关闭通道后台线程和 ROS2。
rclcomm::~rclcomm()
{
    ShutDown();
}

/// 初始化 ROS2 节点、订阅器、发布器、执行器和 TF listener。
bool rclcomm::Start()
{
    rclcpp::init(0,nullptr);
    executor_ = std::make_shared<rclcpp::executors::MultiThreadedExecutor>();
    node_ = std::make_shared<rclcpp::Node>("yxrobot_gui");
    executor_->add_node(node_);
    callback_group_laser_ =
        node_->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    callback_group_other_ =
        node_->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    auto sub1_obt = rclcpp::SubscriptionOptions();
    sub1_obt.callback_group = callback_group_other_;
    auto sub_laser_obt = rclcpp::SubscriptionOptions();
    sub_laser_obt.callback_group = callback_group_laser_;

    globalMap_sub_ = node_->create_subscription<nav_msgs::msg::OccupancyGrid>(
        "map",
        rclcpp::QoS(rclcpp::KeepLast(1)).reliable().transient_local(),
        std::bind(&rclcomm::map_callback,this,std::placeholders::_1),
        sub1_obt);

    globalCostMap_sub_ = node_->create_subscription<nav_msgs::msg::OccupancyGrid>(
        "global_costmap/costmap",
        rclcpp::QoS(rclcpp::KeepLast(1)).reliable().transient_local(),
        std::bind(&rclcomm::globalCostMapCallback,this,std::placeholders::_1),
        sub1_obt);

    laserScan_sub_ = node_->create_subscription<sensor_msgs::msg::LaserScan>(
        "lidar_scan",
        20,
        std::bind(&rclcomm::laserScanCallback,this,std::placeholders::_1),
        sub_laser_obt);

    global_path_sub_ = node_->create_subscription<nav_msgs::msg::Path>(
        "path",
        20,
        std::bind(&rclcomm::globalPathCallback,this,std::placeholders::_1),
        sub1_obt);

    goal_pose_pub_ = node_->create_publisher<geometry_msgs::msg::PoseStamped>(
        "move_base_simple/goal",
        rclcpp::QoS(rclcpp::KeepLast(1)).reliable());
    navigation_zone_marker_pub_ = node_->create_publisher<visualization_msgs::msg::Marker>(
        "yxrobot_gui/navigation_zone_markers",
        rclcpp::QoS(rclcpp::KeepLast(10)).reliable());

    tf_buffer_ = std::make_unique<tf2_ros::Buffer>(node_->get_clock(), std::chrono::seconds(10));
    transform_listener_ =
        std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

    return true;
}

/**
 * @brief rclcomm::Process
 *  ros2执行函数
 */
/// 单次处理 ROS2 回调，并同步机器人 TF 位姿。
void rclcomm::Process()
{
    if(rclcpp::ok())
    {
        getRobotPose();
        executor_->spin_some();
    }
}

/// 关闭 ROS2。
bool rclcomm::Stop()
{
    rclcpp::shutdown();
    return true;
}

/// 将全部导航区域发布为 Marker。
bool rclcomm::SendNavigationZones(const NavigationZoneCollection& zones)
{
    if (!node_) {
        return false;
    }

    return publishZoneMarkers(zones, navigation_zone_marker_pub_, node_->get_clock()->now());
}

/// 将 P2P 任务发布为 RViz/Nav2 兼容的 goal_pose 位姿话题。
bool rclcomm::SendPncTask(const NavigationPncTask& task)
{
    if (!goal_pose_pub_ || task.type != NavigationPncTaskType::P2P || !task.has_goal) {
        return false;
    }

    geometry_msgs::msg::PoseStamped msg;
    msg.header.stamp = node_->get_clock()->now();
    msg.header.frame_id = "map";
    msg.pose.position.x = task.goal.x;
    msg.pose.position.y = task.goal.y;
    msg.pose.position.z = 0.0;
    msg.pose.orientation.x = 0.0;
    msg.pose.orientation.y = 0.0;
    msg.pose.orientation.z = std::sin(task.goal.yaw / 2.0);
    msg.pose.orientation.w = std::cos(task.goal.yaw / 2.0);

    goal_pose_pub_->publish(msg);
    return true;
}

/// 将 ROS2 OccupancyGrid 转换为内部 OccupancyMap 并发给 UI。
void rclcomm::map_callback(const nav_msgs::msg::OccupancyGrid::SharedPtr msg)
{
    // std::cout<<"收到订阅全局地图数据"<<std::endl;
    int width = msg->info.width;
    int height = msg->info.height;
    double originX = msg->info.origin.position.x;
    double originY = msg->info.origin.position.y;
    double resolution = msg->info.resolution;

    OccupancyMap map_data(height,width,resolution,Eigen::Vector3d(originX,originY,0));
    for(size_t i=0;i<msg->data.size();i++){
        int x= int(i/width);
        int y = i%width;
        map_data(x,y) = msg->data[i];
    }
    m_globalMap=std::move(map_data);

    emit emitUpdateMap(m_globalMap);
}

/// 将 ROS2 全局代价地图转换为内部 OccupancyMap。
void rclcomm::globalCostMapCallback(const nav_msgs::msg::OccupancyGrid::SharedPtr msg)
{
    int width = msg->info.width;
    int height = msg->info.height;
    double origin_x = msg->info.origin.position.x;
    double origin_y = msg->info.origin.position.y;
    OccupancyMap cost_map(height, width,msg->info.resolution,Eigen::Vector3d(origin_x, origin_y, 0));
    for (int i = 0; i < msg->data.size(); i++) {
        int x = int(i / width);
        int y = i % width;
        cost_map(x, y) = msg->data[i];
    }
    m_globalCostMap=std::move(cost_map);
    emit emitUpdateGlobalCostMap(m_globalCostMap);
}

/// 局部代价地图回调占位，当前未向 UI 发送数据。
void rclcomm::localCostMapCallback(const nav_msgs::msg::OccupancyGrid::SharedPtr msg)
{
    int width = msg->info.width;
    int height = msg->info.height;
    double origin_x = msg->info.origin.position.x;
    double origin_y = msg->info.origin.position.y;
    tf2::Quaternion q;
    tf2::fromMsg(msg->info.origin.orientation,q);
    tf2::Matrix3x3 mat(q);
    double roll,pitch,yaw;
    mat.getEulerYPR(yaw,pitch,roll);
    double origin_theta = yaw;
    // QImage map_image(width, height, QImage::Format_ARGB32);
    // emit emitUpdateGlobalCostMap(map_image);
}

/// 将 ROS2 LaserScan 点转换到 map 坐标系并发给 UI。
void rclcomm::laserScanCallback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
{
    double angle_min = msg->angle_min;
    double angle_max = msg->angle_max;
    double angle_increment = msg->angle_increment;

    try {
        if (!tf_buffer_->canTransform("map", "lidar_link", tf2::TimePointZero, std::chrono::milliseconds(100)))
            return;

        geometry_msgs::msg::TransformStamped transform =
            tf_buffer_->lookupTransform("map", "lidar_link", tf2::TimePointZero, std::chrono::milliseconds(100));
        tf2::Transform tf2_transform;
        tf2::fromMsg(transform.transform, tf2_transform);
        LaserScan laser_points;
        laser_points.reserve(msg->ranges.size());
        for(int i = 0; i < msg->ranges.size(); i++)
        {
            double angle = msg->angle_min + i * angle_increment;
            tf2::Vector3 point_laser(msg->ranges[i] * cos(angle), msg->ranges[i] * sin(angle), 0.0);
            tf2::Vector3 point_base = tf2_transform * point_laser;
            Point p;
            p.x = point_base.x();
            p.y = point_base.y();
            laser_points.push_back(p);
        }
        laser_points.header.frame_id = "map";
        laser_points.id = 1;
        emit emitUpdateLaserScan(laser_points);
    }
    catch (tf2::TransformException &ex)
    {

    }
}

/// 将 ROS2 Path 转换为内部 Path 数据结构。
void rclcomm::globalPathCallback(const nav_msgs::msg::Path::SharedPtr msg)
{
    Path global_path;
    global_path.header.frame_id = msg->header.frame_id;
    global_path.waypoints.reserve(msg->poses.size());
    for(const auto& pose_stamped : msg->poses)
    {
        Point p;
        p.x = pose_stamped.pose.position.x;
        p.y = pose_stamped.pose.position.y;
        global_path.waypoints.push_back(p);
    }
    emit emitUpdatePath(global_path);
    // std::cout<<"update path!"<<std::endl;

}

/// 查询 base_link 到 map 的位姿并发出机器人位姿信号。
void rclcomm::getRobotPose()
{
    auto pose = getTransform("base_link","map");
    emit emitUpdateRobotPose(pose);
}


/// 从 TF 树中查询 from 到 to 的变换。
RobotPose rclcomm::getTransform(const std::string& from,const std::string& to)
{
    RobotPose pose;
    try {
        if(!tf_buffer_->canTransform(to,from,tf2::TimePointZero,std::chrono::milliseconds(100)))
            return pose;

        geometry_msgs::msg::TransformStamped transform =
            tf_buffer_->lookupTransform(to, from, tf2::TimePointZero, std::chrono::milliseconds(100));
        geometry_msgs::msg::Quaternion msg_quat = transform.transform.rotation;
        // 转换类型
        tf2::Quaternion q;
        tf2::fromMsg(msg_quat, q);
        tf2::Matrix3x3 mat(q);
        double roll, pitch, yaw;
        mat.getRPY(roll, pitch, yaw);

        pose.x = transform.transform.translation.x;
        pose.y = transform.transform.translation.y;
        pose.theta = yaw;
    }
    catch (tf2::TransformException &ex) {

    }

    return pose;
}
