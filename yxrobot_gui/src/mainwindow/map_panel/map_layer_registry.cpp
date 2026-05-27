#include "mainwindow/map_panel/map_layer_registry.h"

namespace silverstar {
namespace map_panel {

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

const QVector<MapLayerEntry>& MapLayerRegistry::layers() const
{
    return layers_;
}

MapLayerBase* MapLayerRegistry::layer(const QString& id) const
{
    for (const MapLayerEntry& entry : layers_) {
        if (entry.id == id) {
            return entry.item;
        }
    }

    return nullptr;
}

void MapLayerRegistry::setVisible(const QString& id, bool visible)
{
    MapLayerBase* item = layer(id);
    if (!item) {
        return;
    }

    item->setVisible(visible);
}

bool MapLayerRegistry::isVisible(const QString& id) const
{
    MapLayerBase* item = layer(id);
    return item ? item->isVisible() : false;
}

}  // namespace map_panel
}  // namespace silverstar
