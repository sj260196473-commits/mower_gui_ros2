#ifndef MAP_LAYER_RUNTIME_H
#define MAP_LAYER_RUNTIME_H

#include <QGraphicsScene>
#include "channel/virtual_channel.h"
#include "mainwindow/map_panel/map_layer_registry.h"

class CostMapItem;
class GridLayerItem;
class LaserItem;
class OccMapItem;
class PathLayerItem;
class RobotPoseItem;

class MapLayerRuntime
{
public:
    void initializeDefaultLayers(QGraphicsScene* scene);
    void bindChannel(VirtualChannel* channel);

    MapLayerRegistry& registry();
    const MapLayerRegistry& registry() const;

    GridLayerItem* gridLayer() const;

private:
    void addLayerToScene(QGraphicsScene* scene, MapLayerBase* layer);
    void resolveDefaultLayerPointers();

    MapLayerRegistry registry_;
    OccMapItem* occMapItem_{nullptr};
    CostMapItem* globalCostMapItem_{nullptr};
    GridLayerItem* gridItem_{nullptr};
    RobotPoseItem* robotPoseItem_{nullptr};
    LaserItem* laserScanItem_{nullptr};
    PathLayerItem* globalPathItem_{nullptr};
};

#endif // MAP_LAYER_RUNTIME_H
