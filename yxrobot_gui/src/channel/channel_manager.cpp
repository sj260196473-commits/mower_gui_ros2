#include "channel_manager.h"

ChannelManager::ChannelManager()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QLibrary library;
    library.setFileName(appDir + "/lib/ros2/libchannel_ros2.so");

    if (!library.load()) {
        current_channel_ = nullptr;
        std::cerr << "Failed to load library: " << library.errorString().toStdString() << std::endl;
    } else {
        typedef VirtualChannel* (*GetChannelInstanceFunc)();
        GetChannelInstanceFunc getChannelInstance = (GetChannelInstanceFunc)library.resolve("GetChannelInstance");
        if (getChannelInstance) {
            current_channel_.reset(getChannelInstance());
        } else {
            current_channel_ = nullptr;
        }
    }
}

ChannelManager::~ChannelManager()
{
    if(current_channel_ != nullptr)
        current_channel_.reset();
}
