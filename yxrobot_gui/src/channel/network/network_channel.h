#ifndef NETWORK_CHANNEL_H
#define NETWORK_CHANNEL_H

#include <cstdint>
#include <string>

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

private:
    bool connectToServer();
    void disconnectFromServer();
    bool receiveExact(void* data, size_t size);
    bool receiveFrame(telemetry_protocol::FrameHeader& header, std::string& payload);
    void handleFrame(const telemetry_protocol::FrameHeader& header, const std::string& payload);
    void handleMapPayload(const std::string& payload);
    void handleLaserPayload(const std::string& payload);
    void handlePosePayload(const std::string& payload);

private:
    std::string host_;
    uint16_t port_{11086};
    int socket_fd_{-1};
    bool connected_{false};
    RobotPose latest_pose_;
    bool has_pose_{false};

    float map_width_ = 0.0f;
    float map_ox_ = 0.0f;
    float map_oy_ = 0.0f;
    bool has_map_info_ = {false};
};

#endif  // NETWORK_CHANNEL_H
