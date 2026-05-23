#ifndef OCCMAP_LAYERITEM_H
#define OCCMAP_LAYERITEM_H

#include <iostream>
#include <QPainter>
#include <QRgb>
#include "mainwindow/map_panel/map_layeritem_virtual.h"
#include "common/common.h"

inline QRgb occMapRgbaForOccupancy(double occupancy)
{
    if (occupancy > 0) {
        const int alpha = static_cast<int>(std::clamp(occupancy * 2.55, 0.0, 255.0));
        return qRgba(0, 0, 0, alpha);
    }
    if (occupancy == -1) {
        return qRgba(160, 160, 160, 255);
    }
    return qRgba(255, 255, 255, 255);
}

class OccMapItem : public MapLayerItemVirtual
{
    Q_OBJECT
public:
    OccMapItem(const QString& id,const QString& name,const int& z,QGraphicsItem* parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;
    QRectF boundingRect() const override;



public slots:
    void updateMap(const OccupancyMap& img);

private:
    QImage m_map_image;
};

#endif // OCCMAP_LAYERITEM_H
