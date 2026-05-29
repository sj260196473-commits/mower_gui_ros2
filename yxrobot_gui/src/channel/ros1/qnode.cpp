#include "qnode.h"

#include <cmath>

qnode::qnode() {
    if(Init())
    {
        std::cout<<"初始化成功";
    }
}

qnode::~qnode()
{
    ShutDown();
}

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

    m_tf_listener = std::make_unique<tf::TransformListener>();

    return true;
}

/**
 * @brief qnode::Process
 *  ros1执行函数
 */
void qnode::Process()
{
    if(ros::ok())
    {
        getRobotPose();
        ros::spinOnce();
    }
}

bool qnode::Stop()
{
    ros::shutdown();
    return true;
}

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

void qnode::getRobotPose()
{
    auto pose = getTransform("base_link","map");
    emit emitUpdateRobotPose(pose);
}


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
