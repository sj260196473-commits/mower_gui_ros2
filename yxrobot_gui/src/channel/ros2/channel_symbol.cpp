#include "rclcomm.h"

extern "C" {
/// 创建 ROS2 通道实例，供 ChannelManager 动态加载。
VirtualChannel* GetChannelInstance()
{
    return new rclcomm();
}

/// 销毁 ROS2 通道实例，避免跨动态库直接 delete 的 ABI 风险。
void DestroyChannelInstance(VirtualChannel* channel)
{
    delete channel;
}
}
