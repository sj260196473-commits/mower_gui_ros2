#ifndef GRID_LAYERITEM_H
#define GRID_LAYERITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QRectF>
#include "map_layeritem_virtual.h"

namespace silverstar {
namespace map_panel {

class GridLayerItem : public MapLayerBase
{
public:
    explicit GridLayerItem(const QString& id,const QString& name,const int& z,QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    void setGridSceneLength(double length);
    void setSceneRect(const QRectF& rect);

private:
    QRectF scene_rect_;
    double grid_scene_length_{0.0};
};

}  // namespace map_panel
}  // namespace silverstar

#endif // GRID_LAYERITEM_H
