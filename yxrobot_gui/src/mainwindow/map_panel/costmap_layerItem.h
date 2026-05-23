#ifndef COSTMAP_LAYERITEM_H
#define COSTMAP_LAYERITEM_H
#include <QPainter>
#include "common/common.h"
#include "mainwindow/map_panel/costmap_color_policy.h"
#include "mainwindow/map_panel/map_layeritem_virtual.h"

class CostMapItem : public MapLayerItemVirtual
{
    Q_OBJECT
public:
    CostMapItem(const QString& id,
                const QString& name,
                const int& z,
                QGraphicsItem* parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QRectF boundingRect() const;

public slots:
    void updateMap(const OccupancyMap& map);

private:
    OccupancyMap m_map;
    QImage m_map_image;
    CostMapColorPolicy color_policy_;

};

#endif // COSTMAP_LAYERITEM_H
