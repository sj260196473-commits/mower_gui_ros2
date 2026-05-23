#ifndef GRID_LAYERITEM_H
#define GRID_LAYERITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QRectF>

class GridLayerItem : public QGraphicsItem
{
public:
    explicit GridLayerItem(QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    void setGridSceneLength(double length);
    void setSceneRect(const QRectF& rect);

private:
    QRectF scene_rect_;
    double grid_scene_length_{0.0};
};

#endif // GRID_LAYERITEM_H
