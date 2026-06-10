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

/// 创建默认图层并保存常用图层裸指针，scene 拥有图层对象生命周期。
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

/// 连接通信信号到各图层，同时连接编辑区域提交到通道发送接口。
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
                         editableZoneItem_,
                         [channel](const NavigationZoneCollection& zones) {
            channel->SendNavigationZones(zones);
        });
    }
}

/// 返回可修改的 registry，供右键菜单切换显隐。
MapLayerRegistry& MapLayerRuntime::registry()
{
    return registry_;
}

/// 返回只读 registry，供外部遍历图层信息。
const MapLayerRegistry& MapLayerRuntime::registry() const
{
    return registry_;
}

/// 返回网格图层指针，供视图根据缩放更新网格大小。
GridLayerItem* MapLayerRuntime::gridLayer() const
{
    return gridItem_;
}

/// 返回编辑区域图层指针，供视图委派区域编辑操作。
EditableZoneLayerItem* MapLayerRuntime::editableZoneLayer() const
{
    return editableZoneItem_;
}

/// 将单个图层加入 scene 并注册到 registry。
void MapLayerRuntime::addLayerToScene(QGraphicsScene* scene, MapLayerBase* layer)
{
    if (!scene || !layer) {
        return;
    }

    scene->addItem(layer);
    registry_.addLayer(layer);
}

/// 通过约定 id 解析默认图层，后续避免重复 dynamic_cast 查找。
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
