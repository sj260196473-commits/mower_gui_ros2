#ifndef COSTMAP_LAYERITEM_H
#define COSTMAP_LAYERITEM_H
#include <QPainter>
#include <QRgb>
#include "common/common.h"
#include "mainwindow/map_panel/map_layeritem_virtual.h"

inline QRgb costMapRgbaForCost(int cost)
{
    if (cost >= 100) {
        return qRgba(0xff, 0x00, 0xff, 50);
    }
    if (cost >= 90) {
        return qRgba(0x66, 0xff, 0xff, 50);
    }
    if (cost >= 70) {
        return qRgba(0xff, 0x00, 0x33, 50);
    }
    if (cost >= 60) {
        return qRgba(0xbe, 0x28, 0x1a, 50);
    }
    if (cost >= 50) {
        return qRgba(0xbe, 0x1f, 0x58, 50);
    }
    if (cost >= 40) {
        return qRgba(0xbe, 0x25, 0x76, 50);
    }
    if (cost >= 30) {
        return qRgba(0xbe, 0x2a, 0x99, 50);
    }
    if (cost >= 20) {
        return qRgba(0xbe, 0x35, 0xb3, 50);
    }
    if (cost >= 10) {
        return qRgba(0xb0, 0x3c, 0xbe, 50);
    }
    return qRgba(0, 0, 0, 0);
}

class CostMapItem : public MapLayerBase
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
    QImage m_map_image;

};

#endif // COSTMAP_LAYERITEM_H
