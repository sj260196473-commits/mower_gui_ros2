#ifndef TELEMETRY_PROTOCOL_H
#define TELEMETRY_PROTOCOL_H

#include <cstddef>
#include <cstdint>

namespace telemetry_protocol {

constexpr uint32_t kFrameMagic = 0x44544252;
constexpr uint16_t kFrameVersion = 1;
constexpr size_t kFrameHeaderSize = 24;

enum class FrameTopic : uint16_t {
    Map = 1,
    LaserScan = 2,
    RobotPose = 3,
};

struct FrameHeader {
    uint32_t magic{0};
    uint16_t version{0};
    uint16_t topic{0};
    uint64_t sequence{0};
    uint32_t payload_size{0};
    uint32_t reserved{0};
};

}  // namespace telemetry_protocol

#endif  // TELEMETRY_PROTOCOL_H
