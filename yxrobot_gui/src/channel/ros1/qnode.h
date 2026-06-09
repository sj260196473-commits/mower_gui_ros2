#ifndef RCLCOMM_H
#define RCLCOMM_H

#include <QObject>
#include <thread>
#include <iostream>
#include <atomic>
#include <memory>
//ROS1
#include <ros/ros.h>
#include <nav_msgs/OccupancyGrid.h>
#include <sensor_msgs/LaserScan.h>
#include <nav_msgs/Path.h>
#include <geometry_msgs/PoseStamped.h>
#include <std_msgs/ColorRGBA.h>
#include <visualization_msgs/Marker.h>
#include <tf/transform_listener.h>
#include <tf/tf.h>

#include "channel/virtual_channel.h"


class qnode:public VirtualChannel
{
    Q_OBJECT
public:
    /// 构造 ROS1 通道实例。
    qnode();

    /// 析构时关闭 ROS1 通道。
    ~qnode();

    /// 初始化 ROS1 节点、订阅器和 TF 监听器。
    bool Start();

    /// 周期执行 ROS1 spinOnce 并查询机器人位姿。
    void Process();

    /// 关闭 ROS1。
    bool Stop();

    /// 发布全部导航区域结构化数据。
    bool SendNavigationZones(const NavigationZoneCollection& zones) override;

    /// 发布 PNC 任务结构化数据。
    bool SendPncTask(const NavigationPncTask& task) override;

private:
    /// 接收全局地图并转换为 OccupancyMap。
    void map_callback(const nav_msgs::OccupancyGridConstPtr& msg);

    /// 接收局部代价地图；当前仅保留接口占位。
    void localCostMapCallback(const nav_msgs::OccupancyGridConstPtr& msg);

    /// 接收全局代价地图并转换为 OccupancyMap。
    void globalCostMapCallback(const nav_msgs::OccupancyGridConstPtr& msg);

    /// 接收激光数据，并通过 TF 转换到 map 坐标系。
    void laserScanCallback(const sensor_msgs::LaserScanConstPtr& msg);

    /// 接收全局路径并转换为 Path。
    void globalPathCallback(const nav_msgs::PathConstPtr& msg);

    /// 查询机器人当前位姿并发出 Qt 信号。
    void getRobotPose();

    /// 查询两个坐标系之间的 TF，并转换为 RobotPose。
    RobotPose getTransform(const std::string& from,const std::string& to);

private:
    std::unique_ptr<ros::NodeHandle> m_node_handle;

    ros::Subscriber m_globalmap_sub;
    ros::Subscriber m_globalcostmap_sub;
    ros::Subscriber m_laserscan_sub;
    ros::Subscriber m_global_path_sub;
    ros::Publisher m_navigation_zone_marker_pub;
    ros::Publisher m_goal_pose_pub;

    std::unique_ptr<tf::TransformListener> m_tf_listener;

    OccupancyMap m_globalMap;
    OccupancyMap m_globalCostMap;
};

#endif // RCLCOMM_H
