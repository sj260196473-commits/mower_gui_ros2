#include "occmap_layerItem.h"

namespace silverstar {
namespace map_panel {

/// 初始化占据地图图层元信息。
OccMapItem::OccMapItem(const QString& id,const QString& name,const int& z,QGraphicsItem* parent)
    :MapLayerBase(id,name,"map",parent)
{
    setZValue(z);
    setAcceptHoverEvents(true);
}

/// 绘制缓存的占据地图图像。
void OccMapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->drawImage(0,0,m_map_image);
}

/// 使用地图图像尺寸作为图层边界。
QRectF OccMapItem::boundingRect() const
{
    return QRectF(0,0,m_map_image.width(),m_map_image.height());
}

/// 将占据栅格数据转换为 ARGB 图像缓存。
void OccMapItem::updateMap(const OccupancyMap& map)
{
    prepareGeometryChange();
    m_map_image = QImage(map.Cols(), map.Rows(), QImage::Format_ARGB32);
    for (int y = 0; y < map.Rows(); y++) {
        auto * row = reinterpret_cast<QRgb *>(m_map_image.scanLine(y));
        for (int x = 0; x < map.Cols(); x++) {
            row[x] = occMapRgbaForOccupancy(map(y, x));
        }
    }
    update();
}

}  // namespace map_panel
}  // namespace silverstar
