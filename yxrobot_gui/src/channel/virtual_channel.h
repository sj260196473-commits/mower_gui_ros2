#ifndef VIRTUAL_CHANNEL_H
#define VIRTUAL_CHANNEL_H
#include <QObject>
#include <thread>
#include <atomic>
#include <iostream>
#include "common/common.h"

class VirtualChannel : public QObject
{
    Q_OBJECT
public:
    VirtualChannel() = default;
    virtual ~VirtualChannel() = default;

    virtual bool Start() = 0;
    virtual void Process() = 0;
    virtual bool Stop() = 0;


    bool Init() {
        if (Start()) {
        std::cout << "start channel success" << std::endl;
        run_flag_ = true;
        process_thread_ = std::thread([this]() {
            while (run_flag_) {
            Process();
            std::this_thread::sleep_for(
                std::chrono::milliseconds(1000 / loop_rate_));
            }
        });
        return true;
        }
        return false;
    }
    
    void ShutDown() {
        run_flag_ = false;
        Stop();
        if (process_thread_.joinable()) {
            process_thread_.join();
        }
    }

signals:
    void emitUpdateMap(const OccupancyMap& map);
    void emitUpdateGlobalCostMap(const OccupancyMap& map);
    void emitUpdateRobotPose(RobotPose pose);
    void emitUpdateLaserScan(const LaserScan& scan);

private:
    std::thread process_thread_;
    std::atomic<bool> run_flag_{false};
    int loop_rate_{30};
};

#endif // VIRTUAL_CHANNEL_H
