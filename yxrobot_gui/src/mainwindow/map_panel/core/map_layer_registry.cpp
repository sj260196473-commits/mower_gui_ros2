#include "mainwindow/map_panel/core/map_layer_registry.h"

namespace silverstar {
namespace map_panel {

/// 把图层加入 registry，供右键菜单和运行时查找使用。
void MapLayerRegistry::addLayer(MapLayerBase* layer)
{
    if (!layer) {
        return;
    }

    if (this->layer(layer->getLayerId())) {
        return;
    }

    layers_.push_back({
        layer->getLayerId(),
        layer->getLayerName(),
        layer->getLayerGroup(),
        layer
    });
}

/// 返回所有图层条目，保持加入 scene 时的顺序。
const QVector<MapLayerEntry>& MapLayerRegistry::layers() const
{
    return layers_;
}

/// 线性查找指定 id 的图层对象。
MapLayerBase* MapLayerRegistry::layer(const QString& id) const
{
    for (const MapLayerEntry& entry : layers_) {
        if (entry.id == id) {
            return entry.item;
        }
    }

    return nullptr;
}

/// 修改图层可见性；找不到 id 时直接忽略。
void MapLayerRegistry::setVisible(const QString& id, bool visible)
{
    MapLayerBase* item = layer(id);
    if (!item) {
        return;
    }

    item->setVisible(visible);
}

/// 查询图层可见性；找不到 id 时返回 false。
bool MapLayerRegistry::isVisible(const QString& id) const
{
    MapLayerBase* item = layer(id);
    return item ? item->isVisible() : false;
}

}  // namespace map_panel
}  // namespace silverstar
