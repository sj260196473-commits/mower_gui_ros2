#ifndef CHANNEL_MANAGER_H
#define CHANNEL_MANAGER_H
#include "channel/virtual_channel.h"
#include <memory>
#include <QLibrary>
#include <QCoreApplication>
#include <iostream>



class ChannelManager
{
public:
    ChannelManager();
    ~ChannelManager();
    VirtualChannel* getChannel()
    {
        return current_channel_.get();
    }

private:
    std::unique_ptr<VirtualChannel> current_channel_;
};

#endif // CHANNEL_MANAGER_H
