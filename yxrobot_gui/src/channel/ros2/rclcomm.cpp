#include "rclcomm.h"

rclcomm::rclcomm() {
    if(Init())
    {
        std::cout<<"初始化成功";
    }
}

rclcomm::~rclcomm()
{
    ShutDown();
}

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
        "scan",
        20,
        std::bind(&rclcomm::laserScanCallback,this,std::placeholders::_1),
        sub_laser_obt);

    global_path_sub_ = node_->create_subscription<nav_msgs::msg::Path>(
        "plan",
        20,
        std::bind(&rclcomm::globalPathCallback,this,std::placeholders::_1),
        sub1_obt);


    tf_buffer_ = std::make_unique<tf2_ros::Buffer>(node_->get_clock(), std::chrono::seconds(10));
    transform_listener_ =
        std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

    return true;
}

/**
 * @brief rclcomm::Process
 *  ros2执行函数
 */
void rclcomm::Process()
{
    if(rclcpp::ok())
    {
        getRobotPose();
        executor_->spin_some();
    }
}

bool rclcomm::Stop()
{
    rclcpp::shutdown();
    return true;
}

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

void rclcomm::laserScanCallback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
{
    double angle_min = msg->angle_min;
    double angle_max = msg->angle_max;
    double angle_increment = msg->angle_increment;

    try {
        if (!tf_buffer_->canTransform("map", "base_scan", tf2::TimePointZero, std::chrono::milliseconds(100)))
            return;

        geometry_msgs::msg::TransformStamped transform =
            tf_buffer_->lookupTransform("map", "base_scan", tf2::TimePointZero, std::chrono::milliseconds(100));
        tf2::Transform tf2_transform;
        tf2::fromMsg(transform.transform, tf2_transform);
        LaserScan laser_points;
        laser_points.reserve(msg->ranges.size());
        for(int i = 0; i < msg->ranges.size(); i++)
        {
            double angle = msg->angle_min + i * angle_increment;
            tf2::Vector3 point_laser(msg->ranges[i] * cos(angle), msg->ranges[i] * sin(angle), 0.0);
            tf2::Vector3 point_base = tf2_transform * point_laser;
            Point p,trans_p;
            p.x = point_base.x();
            p.y = point_base.y();
            m_globalMap.worldPose2Scene(p.x,p.y,trans_p.x,trans_p.y);
            laser_points.push_back(trans_p);
        }
        laser_points.id = 1;
        emit emitUpdateLaserScan(laser_points);
    }
    catch (tf2::TransformException &ex)
    {

    }
}

void rclcomm::globalPathCallback(const nav_msgs::msg::Path::SharedPtr msg)
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
    // std::cout<<"update path!"<<std::endl;

}

void rclcomm::getRobotPose()
{
    auto pose = getTransform("base_link","map");
    emit emitUpdateRobotPose(pose);
}


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

