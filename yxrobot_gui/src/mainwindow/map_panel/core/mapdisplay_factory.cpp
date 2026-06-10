#include "mapdisplay_factory.h"
#include "mainwindow/map_panel/layers/costmap_layerItem.h"
#include "mainwindow/map_panel/layers/editable_zone_layeritem.h"
#include "mainwindow/map_panel/layers/grid_layeritem.h"
#include "mainwindow/map_panel/layers/laser_layeritem.h"
#include "mainwindow/map_panel/layers/occmap_layerItem.h"
#include "mainwindow/map_panel/layers/path_layerItem.h"
#include "mainwindow/map_panel/layers/robotpose_layerItem.h"

namespace silverstar {
namespace map_panel {

/// 构造图层工厂；工厂只负责创建对象，不缓存图层实例。
MapDisplayFactory::MapDisplayFactory() {}

/// 根据枚举创建对应图层，并设置统一的 id、名称和 z 值。
MapLayerBase* MapDisplayFactory::createDisplay(MapDisplayType type)
{
    switch (type) {
    case MapDisplayType::OccupancyMap:
        return new OccMapItem("map.occMap", "Occupancy Map", 0);
    case MapDisplayType::GlobalCostMap:
        return new CostMapItem("map.globalCostMap", "Global Cost Map", 10);
    case MapDisplayType::Grid:
        return new GridLayerItem("grid.grid", "Grid", 12);
    case MapDisplayType::RobotPose:
        return new RobotPoseItem("localization.robot", "Robot Pose", 35);
    case MapDisplayType::LaserScan:
        return new LaserItem("scan.laser", "Laser Scan", 20);
    case MapDisplayType::GlobalPath:
        return new PathLayerItem("plan.globalPath", "Global Path", 25);
    case MapDisplayType::EditableZones:
        return new EditableZoneLayerItem("map.editableZones", "Editable Zones", 30);
    }

    return nullptr;
}

/// 返回当前地图面板启动时需要加入 scene 的默认图层列表。
QVector<MapLayerBase*> MapDisplayFactory::createDefaultDisplays()
{
    return {
        createDisplay(MapDisplayType::OccupancyMap),
        createDisplay(MapDisplayType::GlobalCostMap),
        createDisplay(MapDisplayType::Grid),
        createDisplay(MapDisplayType::RobotPose),
        createDisplay(MapDisplayType::LaserScan),
        createDisplay(MapDisplayType::GlobalPath),
        createDisplay(MapDisplayType::EditableZones)
    };
}

}  // namespace map_panel
}  // namespace silverstar
