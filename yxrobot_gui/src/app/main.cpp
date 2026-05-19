#include "mainwindow/mainwindow.h"

#include <QApplication>

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
