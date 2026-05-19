#include "path_layerItem.h"

PathLayerItem::PathLayerItem(const QString& id,const QString& name,const int& z,QGraphicsItem* parent)
    : MapLayerItemVirtual(id, name, "path", parent)
{
    setZValue(z);
    setAcceptHoverEvents(true);
}

void PathLayerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Implementation for painting the path layer item
}

QRectF PathLayerItem::boundingRect() const
{
    // Implementation for defining the bounding rectangle
}