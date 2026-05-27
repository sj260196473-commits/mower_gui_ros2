#include "mainwindow/map_panel/costmap_layerItem.h"

namespace silverstar {
namespace map_panel {

CostMapItem::CostMapItem(const QString& id,
                        const QString& name,
                        const int& z,
                        QGraphicsItem* parent)
    :MapLayerBase(id,name,"map",parent)
{
    setZValue(z);
}

void CostMapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->drawImage(0,0,m_map_image);
}

QRectF CostMapItem::boundingRect() const
{
    return QRectF(0,0,m_map_image.width(),m_map_image.height());
}

void CostMapItem::updateMap(const OccupancyMap& map)
{
    prepareGeometryChange();
    m_map_image = QImage(map.Cols(), map.Rows(),
                        QImage::Format_ARGB32);
    // map_image_.save("./test.png");
    for (int y = 0; y < map.Rows(); y++) {
        auto * row = reinterpret_cast<QRgb *>(m_map_image.scanLine(y));
        for (int x = 0; x < map.Cols(); x++) {
            row[x] = costMapRgbaForCost(map(y, x));
        }
    }
    update();
}

}  // namespace map_panel
}  // namespace silverstar
