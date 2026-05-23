#include "mainwindow/map_panel/costmap_layerItem.h"

CostMapItem::CostMapItem(const QString& id,
                        const QString& name,
                        const int& z,
                        QGraphicsItem* parent)
    :MapLayerItemVirtual(id,name,"map",parent)
{
    setZValue(z);
}

void CostMapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->drawImage(0,0,m_map_image);
}

QRectF CostMapItem::boundingRect() const
{
    return QRectF(0,0,m_map_image.width(),m_map_image.height());
}

void CostMapItem::updateMap(const OccupancyMap& map)
{
    m_map = map;

    prepareGeometryChange();
    m_map_image = QImage(m_map.Cols(), m_map.Rows(),
                        QImage::Format_ARGB32);
    // map_image_.save("./test.png");
    for (int i = 0; i < m_map.Cols(); i++) {
        for (int j = 0; j < m_map.Rows(); j++) {
            const QColor color = color_policy_.colorForCost(m_map(j, i));
            m_map_image.setPixelColor(i, j, color);
        }
    }
    update();
}

