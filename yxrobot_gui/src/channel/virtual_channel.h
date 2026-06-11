#ifndef VIRTUAL_CHANNEL_H
#define VIRTUAL_CHANNEL_H
#include <QObject>
#include <QString>
#include <algorithm>
#include <thread>
#include <atomic>
#include <iostream>
#include "common/common.h"

class VirtualChannel : public QObject
{
    Q_OBJECT
public:
    /// 构造通道基类，不启动任何后台线程。
    VirtualChannel() = default;

    /// 虚析构，允许通过基类指针释放具体通道。
    virtual ~VirtualChannel() = default;

    /// 启动具体通信后端，由 ROS1/ROS2 插件实现。
    virtual bool Start() = 0;

    /// 通道周期处理函数，由后台线程按 loop_rate_ 调用。
    virtual void Process() = 0;

    /// 停止具体通信后端，由 ROS1/ROS2 插件实现。
    virtual bool Stop() = 0;

    /// 发送导航区域集合；默认通道不支持发送。
    virtual bool SendNavigationZones(const NavigationZoneCollection& zones) {
        (void)zones;
        return false;
    }

    /// 发送 PNC 任务；默认通道不支持发送。
    virtual bool SendPncTask(const NavigationPncTask& task) {
        (void)task;
        return false;
    }

    /// 设置网络遥测目标；非网络通道默认不支持。
    virtual bool SetTelemetryTarget(const QString& host, uint16_t port) {
        (void)host;
        (void)port;
        return false;
    }

    /// 请求建立网络遥测连接；非网络通道默认不支持。
    virtual bool ConnectTelemetry() {
        return false;
    }

    /// 请求断开网络遥测连接；非网络通道默认不支持。
    virtual bool DisconnectTelemetry() {
        return false;
    }

    /// 查询网络遥测连接状态；非网络通道默认未连接。
    virtual bool IsTelemetryConnected() const {
        return false;
    }

    /// 通道后台处理线程频率；普通通道默认 30Hz，网络通道可提高以降低队列延迟。
    virtual int LoopRateHz() const {
        return 30;
    }


    /// 初始化通道并启动后台处理线程。
    bool Init() {
        if (run_flag_) {
            std::cerr << "Channel is already running!" << std::endl;
            return true;
        }

        if (Start()) {
            std::cout << "start channel success" << std::endl;
            run_flag_ = true;
            process_thread_ = std::thread([this]() {
                while (run_flag_) {
                    Process();
                    const int loop_rate = std::max(1, LoopRateHz());
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(1000 / loop_rate));
                }
            });
            return true;
        }
        return false;
    }
    
    /// 停止后台线程并关闭通道后端。
    void ShutDown() {
        if (!run_flag_ && !process_thread_.joinable()) {
            return;
        }

        run_flag_ = false;
        if (process_thread_.joinable()) {
            process_thread_.join();
        }
        Stop();
    }

signals:
    /// 通知 UI 更新全局地图。
    void emitUpdateMap(const OccupancyMap& map);

    /// 通知 UI 更新全局代价地图。
    void emitUpdateGlobalCostMap(const OccupancyMap& map);

    /// 通知 UI 更新机器人位姿。
    void emitUpdateRobotPose(RobotPose pose);

    /// 通知 UI 更新激光点云。
    void emitUpdateLaserScan(const LaserScan& scan);

    /// 通知 UI 更新路径数据。
    void emitUpdatePath(const Path& path);

    /// 通知 UI 网络遥测连接状态变化。
    void emitTelemetryConnectionChanged(bool connected, const QString& message);

    /// 通知 UI 某类遥测数据接收频率变化，单位 Hz。
    void emitTelemetryFrequencyChanged(const QString& topic, double hz);

private:
    std::thread process_thread_;
    std::atomic<bool> run_flag_{false};
    int loop_rate_{30};
};

#endif // VIRTUAL_CHANNEL_H
