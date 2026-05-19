#ifndef PATH_LAYERITEM_H
#define PATH_LAYERITEM_H
#include "mainwindow/map_panel/map_layeritem_virtual.h"
#include "common/common.h"
#include <QPainter>
#include <QColor>
#include <QGraphicsItem>
#include <QGraphicsSceneWheelEvent>

class PathLayerItem : public MapLayerItemVirtual
{
public:
    PathLayerItem(const QString& id,const QString& name,const int& z,QGraphicsItem* parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;
    QRectF boundingRect() const;
};

#endif // PATH_LAYERITEM_H
