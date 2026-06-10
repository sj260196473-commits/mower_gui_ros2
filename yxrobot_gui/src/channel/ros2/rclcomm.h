#ifndef RCLCOMM_H
#define RCLCOMM_H

#include <thread>
#include <iostream>
//ROS2
#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <nav_msgs/msg/path.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <visualization_msgs/msg/marker.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include "tf2_ros/buffer.h"
#include "tf2_ros/transform_listener.h"
#include "tf2/LinearMath/Quaternion.h"

#include "channel/virtual_channel.h"


class rclcomm:public VirtualChannel
{
public:
    /// 构造 ROS2 通道实例。
    rclcomm();

    /// 析构时关闭 ROS2 通道。
    ~rclcomm();

    /// 初始化 ROS2 节点、订阅器、发布器和 TF listener。
    bool Start();

    /// 周期执行 ROS2 executor 并查询机器人位姿。
    void Process();

    /// 关闭 ROS2。
    bool Stop();

    /// 发布全部导航区域结构化数据。
    bool SendNavigationZones(const NavigationZoneCollection& zones) override;

    /// 发布 PNC 任务结构化数据。
    bool SendPncTask(const NavigationPncTask& task) override;

private:
    /// 接收全局地图并转换为 OccupancyMap。
    void map_callback(const nav_msgs::msg::OccupancyGrid::SharedPtr msg);

    /// 接收局部代价地图；当前仅保留接口占位。
    void localCostMapCallback(const nav_msgs::msg::OccupancyGrid::SharedPtr msg);

    /// 接收全局代价地图并转换为 OccupancyMap。
    void globalCostMapCallback(const nav_msgs::msg::OccupancyGrid::SharedPtr msg);

    /// 接收激光数据，并通过 TF 转换到 map 坐标系。
    void laserScanCallback(const sensor_msgs::msg::LaserScan::SharedPtr msg);

    /// 接收全局路径并转换为 Path。
    void globalPathCallback(const nav_msgs::msg::Path::SharedPtr msg);

    /// 查询机器人当前位姿并发出 Qt 信号。
    void getRobotPose();

    /// 查询两个坐标系之间的 TF，并转换为 RobotPose。
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

    rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr navigation_zone_marker_pub_;
    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr goal_pose_pub_;

    std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
    std::shared_ptr<tf2_ros::TransformListener> transform_listener_;

    OccupancyMap m_globalMap;
    OccupancyMap m_globalCostMap;
};

#endif // RCLCOMM_H
