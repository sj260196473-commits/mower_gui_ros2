#ifndef PATH_LAYERITEM_H
#define PATH_LAYERITEM_H
#include <iostream>
#include "mainwindow/map_panel/core/map_layeritem_virtual.h"
#include "common/common.h"
#include "common/map_coordinate_transformer.h"
#include <QPainter>
#include <QColor>
#include <QGraphicsItem>
#include <QGraphicsSceneWheelEvent>

namespace silverstar {
namespace map_panel {

class PathLayerItem : public MapLayerBase
{
    Q_OBJECT
public:
    /// 构造路径图层并设置 z 值。
    PathLayerItem(const QString& id,const QString& name,const int& z,QGraphicsItem* parent = nullptr);

    /// 绘制当前缓存的 scene 坐标路径。
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;

    /// 返回路径包围盒。
    QRectF boundingRect() const;

private:
    /// 根据路径点计算图层包围盒。
    void computeBoundRect(const Path& path);

    /// 绘制路径折线。
    void drawPath(QPainter *painter, const Path& path);

    /// 使用当前地图转换器重建 scene 坐标路径。
    void rebuildScenePath();

public slots:
    /// 更新地图转换参数并重建路径缓存。
    void updateMap(const OccupancyMap& map);

    /// 接收新的世界坐标路径数据。
    void UpdatePath(const Path& path);

private:
    QRectF bounding_rect_;
    MapCoordinateTransformer coordinate_transformer_;
    Path current_path_world_;
    Path current_path_scene_;
};

}  // namespace map_panel
}  // namespace silverstar

#endif // PATH_LAYERITEM_H
