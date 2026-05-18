#ifndef CHANNEL_MANAGER_H
#define CHANNEL_MANAGER_H
#include "channel/virtual_channel.h"
#include <QCoreApplication>
#include <QLibrary>
#include <memory>
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
    using DestroyChannelInstanceFunc = void (*)(VirtualChannel*);

    struct ChannelDeleter {
        DestroyChannelInstanceFunc destroy{nullptr};

        void operator()(VirtualChannel* channel) const
        {
            if (channel != nullptr && destroy != nullptr) {
                destroy(channel);
            }
        }
    };

    std::unique_ptr<VirtualChannel, ChannelDeleter> current_channel_{nullptr, ChannelDeleter{nullptr}};
    QLibrary library_;
};

#endif // CHANNEL_MANAGER_H
