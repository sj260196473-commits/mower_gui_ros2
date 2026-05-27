#include "mapdisplay_factory.h"
#include "mainwindow/map_panel/layers/costmap_layerItem.h"
#include "mainwindow/map_panel/layers/grid_layeritem.h"
#include "mainwindow/map_panel/layers/laser_layeritem.h"
#include "mainwindow/map_panel/layers/occmap_layerItem.h"
#include "mainwindow/map_panel/layers/path_layerItem.h"
#include "mainwindow/map_panel/layers/robotpose_layerItem.h"

namespace silverstar {
namespace map_panel {

MapDisplayFactory::MapDisplayFactory() {}

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
        return new RobotPoseItem("localization.robot", "Robot Pose", 15);
    case MapDisplayType::LaserScan:
        return new LaserItem("scan.laser", "Laser Scan", 20);
    case MapDisplayType::GlobalPath:
        return new PathLayerItem("plan.globalPath", "Global Path", 25);
    }

    return nullptr;
}

QVector<MapLayerBase*> MapDisplayFactory::createDefaultDisplays()
{
    return {
        createDisplay(MapDisplayType::OccupancyMap),
        createDisplay(MapDisplayType::GlobalCostMap),
        createDisplay(MapDisplayType::Grid),
        createDisplay(MapDisplayType::RobotPose),
        createDisplay(MapDisplayType::LaserScan),
        createDisplay(MapDisplayType::GlobalPath)
    };
}

}  // namespace map_panel
}  // namespace silverstar
