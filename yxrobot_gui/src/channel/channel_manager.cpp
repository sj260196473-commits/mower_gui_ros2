#include "channel_manager.h"

#include <QDir>
#include <QString>

//cmake中传过来的中间件通道选项，值为"none"或"ros2"
constexpr const char* kChannelOption = YX_CHANNEL_OPTION;

QString channelLibraryPath(const QString& channelOption)
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString libraryName = QStringLiteral("libchannel_%1.so").arg(channelOption);
    return QDir(appDir).filePath(QStringLiteral("lib/%1/%2").arg(channelOption, libraryName));
}

ChannelManager::ChannelManager()
{
    const QString channelOption = QString::fromUtf8(kChannelOption);

    std::cout << "Selected channel option: " << channelOption.toStdString() << std::endl;

    if (channelOption == QStringLiteral("none")) {
        return;
    }

    library_.setFileName(channelLibraryPath(channelOption));

    if (!library_.load()) 
    {
        current_channel_ = nullptr;
        std::cerr << "Failed to load library: " << library_.errorString().toStdString() << std::endl;
    } 
    else 
    {
        typedef VirtualChannel* (*GetChannelInstanceFunc)();
        typedef void (*DestroyChannelInstanceFunc)(VirtualChannel*);
        GetChannelInstanceFunc getChannelInstance = (GetChannelInstanceFunc)library_.resolve("GetChannelInstance");
        DestroyChannelInstanceFunc destroyChannelInstance = (DestroyChannelInstanceFunc)library_.resolve("DestroyChannelInstance");

        if (getChannelInstance && destroyChannelInstance) 
        {
            current_channel_ = std::unique_ptr<VirtualChannel, ChannelDeleter>(
                getChannelInstance(),
                ChannelDeleter{destroyChannelInstance});
        } 
        else 
        {
            current_channel_ = nullptr;
            std::cerr << "Failed to resolve channel symbols: "
                      << library_.errorString().toStdString() << std::endl;
            library_.unload();
        }
    }
}

ChannelManager::~ChannelManager()
{
    current_channel_.reset();
    library_.unload();
}
