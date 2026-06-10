#ifndef GRID_LAYERITEM_H
#define GRID_LAYERITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QRectF>
#include "mainwindow/map_panel/core/map_layeritem_virtual.h"

namespace silverstar {
namespace map_panel {

class GridLayerItem : public MapLayerBase
{
public:
    /// 构造网格图层并设置 z 值。
    explicit GridLayerItem(const QString& id,const QString& name,const int& z,QGraphicsItem* parent = nullptr);

    /// 返回网格覆盖的 scene 范围。
    QRectF boundingRect() const override;

    /// 按当前网格间距绘制水平和垂直辅助线。
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    /// 设置单个网格在 scene 坐标中的长度。
    void setGridSceneLength(double length);

    /// 设置网格绘制范围。
    void setSceneRect(const QRectF& rect);

private:
    QRectF scene_rect_;
    double grid_scene_length_{0.0};
};

}  // namespace map_panel
}  // namespace silverstar

#endif // GRID_LAYERITEM_H
