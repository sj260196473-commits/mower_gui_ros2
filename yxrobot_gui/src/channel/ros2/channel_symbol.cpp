#include "rclcomm.h"

extern "C" {
VirtualChannel* GetChannelInstance()
{
    return new rclcomm();
}

void DestroyChannelInstance(VirtualChannel* channel)
{
    delete channel;
}
}
