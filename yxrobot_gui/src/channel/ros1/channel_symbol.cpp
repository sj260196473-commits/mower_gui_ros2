#include "qnode.h"

extern "C" {
VirtualChannel* GetChannelInstance()
{
    return new qnode();
}

void DestroyChannelInstance(VirtualChannel* channel)
{
    delete channel;
}
}
