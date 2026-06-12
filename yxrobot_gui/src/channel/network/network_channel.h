#ifndef NETWORK_CHANNEL_H
#define NETWORK_CHANNEL_H

#include <cstdint>
#include <string>
#include <chrono>
#include <mutex>

#include "channel/virtual_channel.h"
#include "telemetry_protocol.h"

namespace {



} // namespace

class NetworkChannel : public VirtualChannel
{
public:
    NetworkChannel();
    ~NetworkChannel() override;

    bool Start() override;
    void Process() override;
    bool Stop() override;
    bool SetTelemetryTarget(const QString& host, uint16_t port) override;
    bool ConnectTelemetry() override;
    bool DisconnectTelemetry() override;
    bool IsTelemetryConnected() const override;
    int LoopRateHz() const override { return 200; }

private:
    bool connectToServer();
    void disconnectFromServer();
    bool receiveExact(void* data, size_t size);
    bool receiveFrame(telemetry_protocol::FrameHeader& header, std::string& payload);
    void handleFrame(const telemetry_protocol::FrameHeader& header, const std::string& payload);
    void handleMapPayload(const std::string& payload);
    void handleLaserPayload(const std::string& payload);
    void handlePosePayload(const std::string& payload);
    void updateFrequency(telemetry_protocol::FrameTopic topic);
    void emitConnectionState(bool connected, const QString& message);

private:
    std::string host_;
    uint16_t port_{11086};
    int socket_fd_{-1};
    bool connected_{false};
    bool connect_enabled_{false};
    bool reconnect_requested_{false};
    mutable std::mutex state_mutex_;
    RobotPose latest_pose_;
    bool has_pose_{false};

    uint32_t map_count_{0};
    uint32_t lidar_count_{0};
    uint32_t pose_count_{0};
    std::chrono::steady_clock::time_point freq_window_start_{std::chrono::steady_clock::now()};

    float map_width_ = 0.0f;
    float map_ox_ = 0.0f;
    float map_oy_ = 0.0f;
    bool has_map_info_ = {false};
};

#endif  // NETWORK_CHANNEL_H
