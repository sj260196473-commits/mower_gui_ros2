#include "occmap_layerItem.h"

namespace silverstar {
namespace map_panel {

OccMapItem::OccMapItem(const QString& id,const QString& name,const int& z,QGraphicsItem* parent)
    :MapLayerBase(id,name,"map",parent)
{
    setZValue(z);
    setAcceptHoverEvents(true);
}

void OccMapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->drawImage(0,0,m_map_image);
}

QRectF OccMapItem::boundingRect() const
{
    return QRectF(0,0,m_map_image.width(),m_map_image.height());
}

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
