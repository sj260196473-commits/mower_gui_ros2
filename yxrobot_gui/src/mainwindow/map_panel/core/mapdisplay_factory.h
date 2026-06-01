#ifndef MAPDISPLAY_FACTORY_H
#define MAPDISPLAY_FACTORY_H

#include <QObject>
#include <QVector>
#include "mainwindow/map_panel/core/map_layeritem_virtual.h"

namespace silverstar {
namespace map_panel {

enum class MapDisplayType
{
    OccupancyMap,
    GlobalCostMap,
    Grid,
    RobotPose,
    LaserScan,
    GlobalPath,
    EditableZones
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

    MapLayerBase* createDisplay(MapDisplayType type);
    QVector<MapLayerBase*> createDefaultDisplays();
};

}  // namespace map_panel
}  // namespace silverstar

#endif // MAPDISPLAY_FACTORY_H
