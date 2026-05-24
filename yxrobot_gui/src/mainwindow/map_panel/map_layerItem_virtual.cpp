#include "mainwindow/map_panel/map_layeritem_virtual.h"

MapLayerBase::MapLayerBase(const QString& layerId,
                           const QString& layerName,
                           const QString& layerGroup,
                           QGraphicsItem* parent)
    :QGraphicsObject(parent),
    m_layerId(layerId),
    m_layerName(layerName),
    m_layerGroup(layerGroup)
{
    // setFlag(QGraphicsItem::ItemIsSelectable, false);
    // setFlag(QGraphicsItem::ItemIsMovable, false);
}
