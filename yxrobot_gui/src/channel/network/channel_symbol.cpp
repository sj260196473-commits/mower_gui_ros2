#include "network_channel.h"

extern "C" {

VirtualChannel* GetChannelInstance()
{
    return new NetworkChannel();
}

void DestroyChannelInstance(VirtualChannel* channel)
{
    delete channel;
}

}
