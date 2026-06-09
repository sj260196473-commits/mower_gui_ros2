#ifndef MAP_LAYER_BASE_H
#define MAP_LAYER_BASE_H
#include <QGraphicsObject>

namespace silverstar {
namespace map_panel {

class MapLayerBase : public QGraphicsObject
{
    Q_OBJECT
public:
    /// 构造地图图层基类，保存图层 id、名称和分组信息。
    explicit MapLayerBase(const QString& layerId,
                          const QString& layerName,
                          const QString& layerGroup,
                          QGraphicsItem* parent = nullptr);

    /// 返回图层唯一 id，用于 registry 查找和显隐控制。
    QString getLayerId() const{return m_layerId;}

    /// 返回图层显示名称，用于右键菜单展示。
    QString getLayerName() const{return m_layerName;}

    /// 返回图层分组名称，用于右键菜单分类。
    QString getLayerGroup() const{return m_layerGroup;}

private:
    QString m_layerId;
    QString m_layerGroup;
    QString m_layerName;
};

}  // namespace map_panel
}  // namespace silverstar

#endif // MAP_LAYER_BASE_H
