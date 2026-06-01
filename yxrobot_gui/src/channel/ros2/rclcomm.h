#ifndef RCLCOMM_H
#define RCLCOMM_H

#include <thread>
#include <iostream>
//ROS2
#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <nav_msgs/msg/path.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <std_msgs/msg/string.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include "tf2_ros/buffer.h"
#include "tf2_ros/transform_listener.h"
#include "tf2/LinearMath/Quaternion.h"

#include "channel/virtual_channel.h"


class rclcomm:public VirtualChannel
{
public:
    rclcomm();
    ~rclcomm();
    bool Start();
    void Process();
    bool Stop();
    bool SendPlanningZones(const QString& zones_json) override;
    bool SendBlockedAreas(const QString& blocked_areas_json) override;
    bool SendPncTask(const QString& task_json) override;

private:
    void map_callback(const nav_msgs::msg::OccupancyGrid::SharedPtr msg);
    void localCostMapCallback(const nav_msgs::msg::OccupancyGrid::SharedPtr msg);
    void globalCostMapCallback(const nav_msgs::msg::OccupancyGrid::SharedPtr msg);
    void laserScanCallback(const sensor_msgs::msg::LaserScan::SharedPtr msg);
    void globalPathCallback(const nav_msgs::msg::Path::SharedPtr msg);
    void getRobotPose();
    RobotPose getTransform(const std::string& from,const std::string& to);

private:
    rclcpp::executors::MultiThreadedExecutor::SharedPtr executor_;
    std::shared_ptr<rclcpp::Node> node_;
    rclcpp::CallbackGroup::SharedPtr callback_group_laser_;
    rclcpp::CallbackGroup::SharedPtr callback_group_other_;

    rclcpp::Subscription<nav_msgs::msg::OccupancyGrid>::SharedPtr globalMap_sub_;
    rclcpp::Subscription<nav_msgs::msg::OccupancyGrid>::SharedPtr globalCostMap_sub_;
    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr laserScan_sub_;
    rclcpp::Subscription<nav_msgs::msg::Path>::SharedPtr global_path_sub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr planning_zones_pub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr blocked_areas_pub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pnc_task_pub_;

    std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
    std::shared_ptr<tf2_ros::TransformListener> transform_listener_;

    OccupancyMap m_globalMap;
    OccupancyMap m_globalCostMap;
};

#endif // RCLCOMM_H
