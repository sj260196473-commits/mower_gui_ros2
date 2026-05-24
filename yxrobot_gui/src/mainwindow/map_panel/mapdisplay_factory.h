#ifndef MAPDISPLAY_FACTORY_H
#define MAPDISPLAY_FACTORY_H

#include <QObject>
#include <QVector>
#include "mainwindow/map_panel/map_layeritem_virtual.h"

enum class MapDisplayType
{
    OccupancyMap,
    GlobalCostMap,
    Grid,
    RobotPose,
    LaserScan,
    GlobalPath
};

class MapDisplayFactory : public QObject
{
    Q_OBJECT
public:
    static MapDisplayFactory *Instance(){
        static MapDisplayFactory *factory = new MapDisplayFactory();
        return factory;
    }
    MapDisplayFactory();

    MapLayerItemVirtual* createDisplay(MapDisplayType type);
    QVector<MapLayerItemVirtual*> createDefaultDisplays();
};

#endif // MAPDISPLAY_FACTORY_H
