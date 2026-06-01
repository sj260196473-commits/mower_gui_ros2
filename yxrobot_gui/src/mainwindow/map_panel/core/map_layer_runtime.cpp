#include "mainwindow/map_panel/core/map_layer_runtime.h"

#include <QObject>
#include "mainwindow/map_panel/core/mapdisplay_factory.h"
#include "mainwindow/map_panel/layers/costmap_layerItem.h"
#include "mainwindow/map_panel/layers/editable_zone_layeritem.h"
#include "mainwindow/map_panel/layers/grid_layeritem.h"
#include "mainwindow/map_panel/layers/laser_layeritem.h"
#include "mainwindow/map_panel/layers/occmap_layerItem.h"
#include "mainwindow/map_panel/layers/path_layerItem.h"
#include "mainwindow/map_panel/layers/robotpose_layerItem.h"

namespace silverstar {
namespace map_panel {

void MapLayerRuntime::initializeDefaultLayers(QGraphicsScene* scene)
{
    if (!scene) {
        return;
    }

    auto* factory = MapDisplayFactory::Instance();
    for (MapLayerBase* layer : factory->createDefaultDisplays()) {
        addLayerToScene(scene, layer);
    }

    resolveDefaultLayerPointers();
}

void MapLayerRuntime::bindChannel(VirtualChannel* channel)
{
    if (!channel) {
        return;
    }

    if (occMapItem_) {
        QObject::connect(channel, &VirtualChannel::emitUpdateMap,
                         occMapItem_, &OccMapItem::updateMap, Qt::QueuedConnection);
    }
    if (robotPoseItem_) {
        QObject::connect(channel, &VirtualChannel::emitUpdateMap,
                         robotPoseItem_, &RobotPoseItem::updateMap, Qt::QueuedConnection);
        QObject::connect(channel, &VirtualChannel::emitUpdateRobotPose,
                         robotPoseItem_, &RobotPoseItem::updatePose, Qt::QueuedConnection);
    }
    if (laserScanItem_) {
        QObject::connect(channel, &VirtualChannel::emitUpdateMap,
                         laserScanItem_, &LaserItem::updateMap, Qt::QueuedConnection);
        QObject::connect(channel, &VirtualChannel::emitUpdateLaserScan,
                         laserScanItem_, &LaserItem::UpdateLaserData, Qt::QueuedConnection);
    }
    if (globalPathItem_) {
        QObject::connect(channel, &VirtualChannel::emitUpdateMap,
                         globalPathItem_, &PathLayerItem::updateMap, Qt::QueuedConnection);
        QObject::connect(channel, &VirtualChannel::emitUpdatePath,
                         globalPathItem_, &PathLayerItem::UpdatePath, Qt::QueuedConnection);
    }
    if (globalCostMapItem_) {
        QObject::connect(channel, &VirtualChannel::emitUpdateGlobalCostMap,
                         globalCostMapItem_, &CostMapItem::updateMap, Qt::QueuedConnection);
    }
    if (editableZoneItem_) {
        QObject::connect(channel, &VirtualChannel::emitUpdateMap,
                         editableZoneItem_, &EditableZoneLayerItem::updateMap, Qt::QueuedConnection);
        QObject::connect(editableZoneItem_, &EditableZoneLayerItem::zoneCommitRequested,
                         editableZoneItem_, [channel](const QString& planningZonesJson, const QString& blockedAreasJson) {
            channel->SendPlanningZones(planningZonesJson);
            channel->SendBlockedAreas(blockedAreasJson);
        });
    }
}

MapLayerRegistry& MapLayerRuntime::registry()
{
    return registry_;
}

const MapLayerRegistry& MapLayerRuntime::registry() const
{
    return registry_;
}

GridLayerItem* MapLayerRuntime::gridLayer() const
{
    return gridItem_;
}

EditableZoneLayerItem* MapLayerRuntime::editableZoneLayer() const
{
    return editableZoneItem_;
}

void MapLayerRuntime::addLayerToScene(QGraphicsScene* scene, MapLayerBase* layer)
{
    if (!scene || !layer) {
        return;
    }

    scene->addItem(layer);
    registry_.addLayer(layer);
}

void MapLayerRuntime::resolveDefaultLayerPointers()
{
    occMapItem_ = registry_.layerAs<OccMapItem>("map.occMap");
    globalCostMapItem_ = registry_.layerAs<CostMapItem>("map.globalCostMap");
    gridItem_ = registry_.layerAs<GridLayerItem>("grid.grid");
    robotPoseItem_ = registry_.layerAs<RobotPoseItem>("localization.robot");
    laserScanItem_ = registry_.layerAs<LaserItem>("scan.laser");
    globalPathItem_ = registry_.layerAs<PathLayerItem>("plan.globalPath");
    editableZoneItem_ = registry_.layerAs<EditableZoneLayerItem>("map.editableZones");
}

}  // namespace map_panel
}  // namespace silverstar
