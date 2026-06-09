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
    /// 根据编译期通道选项加载对应通信插件。
    ChannelManager();

    /// 释放当前通道实例，并卸载动态库。
    ~ChannelManager();

    /// 返回当前通信通道实例；未启用或加载失败时返回 nullptr。
    VirtualChannel* getChannel()
    {
        return current_channel_.get();
    }

private:
    using DestroyChannelInstanceFunc = void (*)(VirtualChannel*);

    struct ChannelDeleter {
        DestroyChannelInstanceFunc destroy{nullptr};

        /// 调用插件导出的销毁函数释放通道实例。
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
