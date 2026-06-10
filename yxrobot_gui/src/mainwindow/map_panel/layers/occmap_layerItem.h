#ifndef OCCMAP_LAYERITEM_H
#define OCCMAP_LAYERITEM_H

#include <iostream>
#include <QPainter>
#include <QRgb>
#include "mainwindow/map_panel/core/map_layeritem_virtual.h"
#include "common/common.h"

namespace silverstar {
namespace map_panel {

/// 根据占据概率返回地图像素颜色。
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

class OccMapItem : public MapLayerBase
{
    Q_OBJECT
public:
    /// 构造占据地图图层并设置 z 值。
    OccMapItem(const QString& id,const QString& name,const int& z,QGraphicsItem* parent = nullptr);

    /// 绘制缓存的占据地图图像。
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;

    /// 返回占据地图图像包围盒。
    QRectF boundingRect() const override;



public slots:
    /// 接收占据地图数据并转换为 QImage 缓存。
    void updateMap(const OccupancyMap& img);

private:
    QImage m_map_image;
};

}  // namespace map_panel
}  // namespace silverstar

#endif // OCCMAP_LAYERITEM_H
