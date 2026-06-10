#ifndef MAP_LAYER_RUNTIME_H
#define MAP_LAYER_RUNTIME_H

#include <QGraphicsScene>
#include "channel/virtual_channel.h"
#include "mainwindow/map_panel/core/map_layer_registry.h"

namespace silverstar {
namespace map_panel {

class CostMapItem;
class EditableZoneLayerItem;
class GridLayerItem;
class LaserItem;
class OccMapItem;
class PathLayerItem;
class RobotPoseItem;

class MapLayerRuntime
{
public:
    /// 创建默认图层、加入场景并缓存常用图层指针。
    void initializeDefaultLayers(QGraphicsScene* scene);

    /// 将通信通道信号连接到各图层更新槽和命令发送接口。
    void bindChannel(VirtualChannel* channel);

    /// 返回可修改的图层注册表。
    MapLayerRegistry& registry();

    /// 返回只读图层注册表。
    const MapLayerRegistry& registry() const;

    /// 返回网格图层，供视图更新网格范围和尺度。
    GridLayerItem* gridLayer() const;

    /// 返回编辑区域图层，供视图处理区域编辑交互。
    EditableZoneLayerItem* editableZoneLayer() const;

private:
    /// 将图层加入 QGraphicsScene 并同步注册到 registry。
    void addLayerToScene(QGraphicsScene* scene, MapLayerBase* layer);

    /// 根据约定 id 从 registry 中解析默认图层指针。
    void resolveDefaultLayerPointers();

    MapLayerRegistry registry_;
    OccMapItem* occMapItem_{nullptr};
    CostMapItem* globalCostMapItem_{nullptr};
    GridLayerItem* gridItem_{nullptr};
    RobotPoseItem* robotPoseItem_{nullptr};
    LaserItem* laserScanItem_{nullptr};
    PathLayerItem* globalPathItem_{nullptr};
    EditableZoneLayerItem* editableZoneItem_{nullptr};
};

}  // namespace map_panel
}  // namespace silverstar

#endif // MAP_LAYER_RUNTIME_H
