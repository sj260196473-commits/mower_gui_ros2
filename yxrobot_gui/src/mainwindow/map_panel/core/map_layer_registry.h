#ifndef MAP_LAYER_REGISTRY_H
#define MAP_LAYER_REGISTRY_H

#include <QVector>
#include <QString>
#include "mainwindow/map_panel/core/map_layeritem_virtual.h"

namespace silverstar {
namespace map_panel {

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
    /// 注册图层元信息，重复 id 会被忽略。
    void addLayer(MapLayerBase* layer);

    /// 返回所有已注册图层，用于菜单展示和统一遍历。
    const QVector<MapLayerEntry>& layers() const;

    /// 根据图层 id 查找图层对象，找不到返回 nullptr。
    MapLayerBase* layer(const QString& id) const;

    template<typename T>
    /// 根据 id 查找图层并转换为指定派生类型，类型不匹配时返回 nullptr。
    T* layerAs(const QString& id) const
    {
        return dynamic_cast<T*>(layer(id));
    }

    /// 设置指定图层显隐状态。
    void setVisible(const QString& id, bool visible);

    /// 查询指定图层当前是否可见。
    bool isVisible(const QString& id) const;

private:
    QVector<MapLayerEntry> layers_;
};

}  // namespace map_panel
}  // namespace silverstar

#endif // MAP_LAYER_REGISTRY_H
