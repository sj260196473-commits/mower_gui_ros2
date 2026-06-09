#include "mainwindow/mainwindow.h"

#include <QApplication>

/// 应用入口：初始化 Qt、注册跨线程消息类型，并显示主窗口。
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<RobotPose>("RobotPose");
    qRegisterMetaType<OccupancyMap>("OccupancyMap");
    qRegisterMetaType<LaserScan>("LaserScan");
    qRegisterMetaType<Path>("Path");
    MainWindow w;
    w.show();
    return a.exec();
}
