#ifndef MAP_LAYER_REGISTRY_H
#define MAP_LAYER_REGISTRY_H

#include <QVector>
#include <QString>
#include "mainwindow/map_panel/map_layeritem_virtual.h"

struct MapLayerEntry
{
    QString id;
    QString name;
    QString group;
    MapLayerBase* item{nullptr};
};

class MapLayerRegistry
{
public:
    void addLayer(MapLayerBase* layer);

    const QVector<MapLayerEntry>& layers() const;
    MapLayerBase* layer(const QString& id) const;

    template<typename T>
    T* layerAs(const QString& id) const
    {
        return dynamic_cast<T*>(layer(id));
    }

    void setVisible(const QString& id, bool visible);
    bool isVisible(const QString& id) const;

private:
    QVector<MapLayerEntry> layers_;
};

#endif // MAP_LAYER_REGISTRY_H
