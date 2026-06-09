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
    /// 返回地图显示工厂单例，用于集中创建默认图层。
    static MapDisplayFactory *Instance(){
        static MapDisplayFactory *factory = new MapDisplayFactory();
        return factory;
    }

    /// 构造工厂对象，本身不持有图层状态。
    MapDisplayFactory();

    /// 根据图层类型创建一个具体 MapLayerBase 派生对象。
    MapLayerBase* createDisplay(MapDisplayType type);

    /// 创建上位机默认启用的全部地图图层。
    QVector<MapLayerBase*> createDefaultDisplays();
};

}  // namespace map_panel
}  // namespace silverstar

#endif // MAPDISPLAY_FACTORY_H
