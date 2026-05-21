#ifndef PATH_LAYERITEM_H
#define PATH_LAYERITEM_H
#include <iostream>
#include "mainwindow/map_panel/map_layeritem_virtual.h"
#include "common/common.h"
#include <QPainter>
#include <QColor>
#include <QGraphicsItem>
#include <QGraphicsSceneWheelEvent>

class PathLayerItem : public MapLayerItemVirtual
{
    Q_OBJECT
public:
    PathLayerItem(const QString& id,const QString& name,const int& z,QGraphicsItem* parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;
    QRectF boundingRect() const;

private:
    void computeBoundRect(const Path& path);
    void drawPath(QPainter *painter, const Path& path);

public slots:
    void UpdatePath(const Path& path);

private:
    QRectF bounding_rect_;
    Path current_path_;
};

#endif // PATH_LAYERITEM_H
