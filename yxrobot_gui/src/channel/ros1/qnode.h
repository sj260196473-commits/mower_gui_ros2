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
#include <tf/transform_listener.h>
#include <tf/tf.h>

#include "channel/virtual_channel.h"


class qnode:public VirtualChannel
{
    Q_OBJECT
public:
    qnode();
    ~qnode();
    bool Start();
    void Process();
    bool Stop();

private:
    void map_callback(const nav_msgs::OccupancyGridConstPtr& msg);
    void localCostMapCallback(const nav_msgs::OccupancyGridConstPtr& msg);
    void globalCostMapCallback(const nav_msgs::OccupancyGridConstPtr& msg);
    void laserScanCallback(const sensor_msgs::LaserScanConstPtr& msg);
    void globalPathCallback(const nav_msgs::PathConstPtr& msg);
    void getRobotPose();
    RobotPose getTransform(const std::string& from,const std::string& to);

private:
    std::unique_ptr<ros::NodeHandle> m_node_handle;

    ros::Subscriber m_globalmap_sub;
    ros::Subscriber m_globalcostmap_sub;
    ros::Subscriber m_laserscan_sub;
    ros::Subscriber m_global_path_sub;

    std::unique_ptr<tf::TransformListener> m_tf_listener;

    OccupancyMap m_globalMap;
    OccupancyMap m_globalCostMap;
};

#endif // RCLCOMM_H
