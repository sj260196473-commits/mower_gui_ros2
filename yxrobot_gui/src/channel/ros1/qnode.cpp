#include "qnode.h"

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
std_msgs::ColorRGBA markerColor(NavigationZoneKind kind)
{
    std_msgs::ColorRGBA color;
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
void appendMarkerPoint(visualization_msgs::Marker& marker, const NavigationPoint2D& point)
{
    geometry_msgs::Point rosPoint;
    rosPoint.x = point.x;
    rosPoint.y = point.y;
    rosPoint.z = 0.0;
    marker.points.push_back(rosPoint);
}

/// 将通用导航区域转换为 ROS1 Marker 消息。
visualization_msgs::Marker toZoneMarker(const NavigationZone& zone, const ros::Time& stamp)
{
    visualization_msgs::Marker marker;
    marker.header.stamp = stamp;
    marker.header.frame_id = "map";
    marker.ns = markerNamespace(zone.kind);
    marker.id = static_cast<int>(std::hash<std::string>{}(zone.id) & 0x7FFFFFFF);
    marker.type = visualization_msgs::Marker::LINE_STRIP;
    marker.action = visualization_msgs::Marker::ADD;
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
void publishDeleteAllMarkers(ros::Publisher& publisher, const ros::Time& stamp)
{
    visualization_msgs::Marker marker;
    marker.header.stamp = stamp;
    marker.header.frame_id = "map";
    marker.action = visualization_msgs::Marker::DELETEALL;
    publisher.publish(marker);
}

/// 将区域集合逐个发布为 Marker。
bool publishZoneMarkers(
    const NavigationZoneCollection& zones,
    ros::Publisher& publisher,
    const ros::Time& stamp)
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

        publisher.publish(toZoneMarker(zone, stamp));
        published = true;
    }

    return published || zones.zones.empty();
}
}

/// 构造 ROS1 通道，并尝试初始化连接。
qnode::qnode() {
    if(Init())
    {
        std::cout<<"初始化成功";
    }
}

/// 析构时停止后台通道。
qnode::~qnode()
{
    ShutDown();
}

/// 初始化 ROS1、等待 master、创建订阅器和 TF listener。
bool qnode::Start()
{
    int argc = 0;
    ros::init(argc, nullptr, "yxrobot_gui", ros::init_options::AnonymousName);
    while (!ros::master::check()) {
        std::cerr<<"The Ros Master is not running, please check the ros master or run 'roscore' first, waitting for 300ms...";
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    ros::start();

    m_node_handle = std::make_unique<ros::NodeHandle>();
    m_globalmap_sub = m_node_handle->subscribe(
        "map", 
        1, 
        &qnode::map_callback, 
        this);
        
    m_globalcostmap_sub = m_node_handle->subscribe(
        "global_costmap/costmap",
        1,
        &qnode::globalCostMapCallback,
        this);

    m_laserscan_sub = m_node_handle->subscribe(
        "lidar_scan",
        20,
        &qnode::laserScanCallback,
        this);

    m_global_path_sub = m_node_handle->subscribe(
        "path",
        20,
        &qnode::globalPathCallback,
        this);

    m_navigation_zone_marker_pub =
        m_node_handle->advertise<visualization_msgs::Marker>(
            "yxrobot_gui/navigation_zone_markers",
            10);
    m_goal_pose_pub =
        m_node_handle->advertise<geometry_msgs::PoseStamped>(
            "goal_pose",
            1);

    m_tf_listener = std::make_unique<tf::TransformListener>();

    return true;
}

/**
 * @brief qnode::Process
 *  ros1执行函数
 */
/// 单次处理 ROS 回调，并同步机器人 TF 位姿。
void qnode::Process()
{
    if(ros::ok())
    {
        getRobotPose();
        ros::spinOnce();
    }
}

/// 关闭 ROS1 通信。
bool qnode::Stop()
{
    ros::shutdown();
    return true;
}

/// 将全部导航区域发布为 Marker。
bool qnode::SendNavigationZones(const NavigationZoneCollection& zones)
{
    if (!m_node_handle) {
        return false;
    }

    return publishZoneMarkers(zones, m_navigation_zone_marker_pub, ros::Time::now());
}

/// 将 P2P 任务发布为 ROS1 goal_pose 位姿话题。
bool qnode::SendPncTask(const NavigationPncTask& task)
{
    if (!m_goal_pose_pub || task.type != NavigationPncTaskType::P2P || !task.has_goal) {
        return false;
    }

    geometry_msgs::PoseStamped msg;
    msg.header.stamp = ros::Time::now();
    msg.header.frame_id = "map";
    msg.pose.position.x = task.goal.x;
    msg.pose.position.y = task.goal.y;
    msg.pose.position.z = 0.0;
    msg.pose.orientation = tf::createQuaternionMsgFromYaw(task.goal.yaw);
    m_goal_pose_pub.publish(msg);
    return true;
}

/// 将 ROS1 OccupancyGrid 转换为内部 OccupancyMap 并发给 UI。
void qnode::map_callback(const nav_msgs::OccupancyGridConstPtr& msg)
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

/// 将 ROS1 全局代价地图转换为内部 OccupancyMap。
void qnode::globalCostMapCallback(const nav_msgs::OccupancyGridConstPtr& msg)
{
    int width = msg->info.width;
    int height = msg->info.height;
    double origin_x = msg->info.origin.position.x;
    double origin_y = msg->info.origin.position.y;
    OccupancyMap cost_map(height, width,msg->info.resolution,Eigen::Vector3d(origin_x, origin_y, 0));
    for (size_t i = 0; i < msg->data.size(); i++) {
        int x = int(i / width);
        int y = int(i % width);
        cost_map(x, y) = msg->data[i];
    }
    m_globalCostMap=std::move(cost_map);
    emit emitUpdateGlobalCostMap(m_globalCostMap);
}

/// 局部代价地图回调占位，当前未向 UI 发送数据。
void qnode::localCostMapCallback(const nav_msgs::OccupancyGridConstPtr& msg)
{
    const int width = msg->info.width;
    const int height = msg->info.height;
    const double origin_x = msg->info.origin.position.x;
    const double origin_y = msg->info.origin.position.y;
    const double origin_theta = tf::getYaw(msg->info.origin.orientation);
    (void)width;
    (void)height;
    (void)origin_x;
    (void)origin_y;
    (void)origin_theta;
    // QImage map_image(width, height, QImage::Format_ARGB32);
    // emit emitUpdateGlobalCostMap(map_image);
}

/// 将 ROS1 LaserScan 点转换到 map 坐标系并发给 UI。
void qnode::laserScanCallback(const sensor_msgs::LaserScanConstPtr& msg)
{
    double angle_increment = msg->angle_increment;

    try {
        if (!m_tf_listener->waitForTransform("map", "lidar_link", ros::Time(0), ros::Duration(0.1)))
            return;

        tf::StampedTransform transform;
        m_tf_listener->lookupTransform("map", "lidar_link", ros::Time(0), transform);
        LaserScan laser_points;
        laser_points.reserve(msg->ranges.size());
        for(size_t i = 0; i < msg->ranges.size(); i++)
        {
            double angle = msg->angle_min + i * angle_increment;
            tf::Vector3 point_laser(msg->ranges[i] * std::cos(angle), msg->ranges[i] * std::sin(angle), 0.0);
            tf::Vector3 point_base = transform * point_laser;
            Point p;
            p.x = point_base.x();
            p.y = point_base.y();
            laser_points.push_back(p);
        }
        laser_points.header.frame_id = "map";
        laser_points.id = 1;
        emit emitUpdateLaserScan(laser_points);
        // std::cout<<"LaserScan received!"<<std::endl;
    }
    catch (tf::TransformException &ex)
    {

    }
}

/// 将 ROS1 Path 转换为内部 Path 数据结构。
void qnode::globalPathCallback(const nav_msgs::PathConstPtr& msg)
{
    Path global_path;
    global_path.waypoints.reserve(msg->poses.size());
    for(const auto& pose_stamped : msg->poses)
    {
        Point p;    
        p.x = pose_stamped.pose.position.x;
        p.y = pose_stamped.pose.position.y;
        global_path.waypoints.push_back(p);
    }
    emit emitUpdatePath(global_path);
}

/// 查询 base_link 到 map 的位姿并发出机器人位姿信号。
void qnode::getRobotPose()
{
    auto pose = getTransform("base_link","map");
    emit emitUpdateRobotPose(pose);
}


/// 从 TF 树中查询 from 到 to 的变换。
RobotPose qnode::getTransform(const std::string& from,const std::string& to)
{
    RobotPose pose;
    try {
        if(!m_tf_listener->waitForTransform(to, from, ros::Time(0), ros::Duration(0.1)))
            return pose;

        tf::StampedTransform transform;
        m_tf_listener->lookupTransform(to, from, ros::Time(0), transform);

        pose.x = transform.getOrigin().x();
        pose.y = transform.getOrigin().y();
        pose.theta = tf::getYaw(transform.getRotation());
    }
    catch (tf::TransformException &ex) {

    }

    return pose;
}
