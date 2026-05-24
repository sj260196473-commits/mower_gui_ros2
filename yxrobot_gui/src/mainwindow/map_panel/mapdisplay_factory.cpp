#include "mapdisplay_factory.h"
#include "mainwindow/map_panel/costmap_layerItem.h"
#include "mainwindow/map_panel/grid_layeritem.h"
#include "mainwindow/map_panel/laser_layeritem.h"
#include "mainwindow/map_panel/occmap_layerItem.h"
#include "mainwindow/map_panel/path_layerItem.h"
#include "mainwindow/map_panel/robotpose_layerItem.h"

MapDisplayFactory::MapDisplayFactory() {}

MapLayerItemVirtual* MapDisplayFactory::createDisplay(MapDisplayType type)
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

QVector<MapLayerItemVirtual*> MapDisplayFactory::createDefaultDisplays()
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
