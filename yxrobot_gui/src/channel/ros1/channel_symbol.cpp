#include "qnode.h"

extern "C" {
/// 创建 ROS1 通道实例，供 ChannelManager 动态加载。
VirtualChannel* GetChannelInstance()
{
    return new qnode();
}

/// 销毁 ROS1 通道实例，避免跨动态库直接 delete 的 ABI 风险。
void DestroyChannelInstance(VirtualChannel* channel)
{
    delete channel;
}
}
