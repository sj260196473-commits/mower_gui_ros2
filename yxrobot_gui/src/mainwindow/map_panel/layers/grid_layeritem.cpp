#include "mainwindow/map_panel/layers/grid_layeritem.h"

#include <QStyleOptionGraphicsItem>
#include <algorithm>
#include <cmath>

namespace silverstar {
namespace map_panel {

/// 初始化网格图层元信息和 z 值。
GridLayerItem::GridLayerItem(const QString& id,const QString& name,const int& z,QGraphicsItem* parent)
    : MapLayerBase(id, name, "grid", parent)
{
    setZValue(z);
    setAcceptHoverEvents(true);
}

/// 返回网格覆盖区域。
QRectF GridLayerItem::boundingRect() const
{
    return scene_rect_;
}

/// 根据当前 scene 范围和网格间距绘制横竖线。
void GridLayerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    if (scene_rect_.isNull() || scene_rect_.isEmpty() || grid_scene_length_ <= 0.0) {
        return;
    }

    const QRectF visibleRect = option ? option->exposedRect.intersected(scene_rect_) : scene_rect_;
    if (visibleRect.isNull() || visibleRect.isEmpty()) {
        return;
    }

    painter->save();

    QPen gridPen(QColor(210, 210, 210, 180));
    gridPen.setCosmetic(true);
    gridPen.setWidth(1);
    painter->setPen(gridPen);

    const double left = std::floor(visibleRect.left() / grid_scene_length_) * grid_scene_length_;
    const double right = visibleRect.right();
    const double top = std::floor(visibleRect.top() / grid_scene_length_) * grid_scene_length_;
    const double bottom = visibleRect.bottom();

    for (double x = left; x <= right; x += grid_scene_length_) {
        painter->drawLine(QPointF(x, visibleRect.top()), QPointF(x, visibleRect.bottom()));
    }

    for (double y = top; y <= bottom; y += grid_scene_length_) {
        painter->drawLine(QPointF(visibleRect.left(), y), QPointF(visibleRect.right(), y));
    }

    QPen axisPen(QColor(140, 140, 140, 220));
    axisPen.setCosmetic(true);
    axisPen.setWidth(1);
    painter->setPen(axisPen);
    painter->drawLine(QPointF(0.0, visibleRect.top()), QPointF(0.0, visibleRect.bottom()));
    painter->drawLine(QPointF(visibleRect.left(), 0.0), QPointF(visibleRect.right(), 0.0));

    painter->restore();
}

/// 更新网格间距，非法长度会使网格不绘制。
void GridLayerItem::setGridSceneLength(double length)
{
    if (std::abs(length - grid_scene_length_) < 1e-9) {
        return;
    }

    grid_scene_length_ = length;
    update();
}

/// 更新网格覆盖范围并触发重绘。
void GridLayerItem::setSceneRect(const QRectF& rect)
{
    if (scene_rect_ == rect) {
        return;
    }

    prepareGeometryChange();
    scene_rect_ = rect;
    update();
}

}  // namespace map_panel
}  // namespace silverstar
