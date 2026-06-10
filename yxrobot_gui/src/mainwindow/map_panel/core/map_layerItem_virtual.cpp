#include "mainwindow/map_panel/core/map_layeritem_virtual.h"

namespace silverstar {
namespace map_panel {

/// 初始化图层元信息，实际绘制由派生类实现。
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

}  // namespace map_panel
}  // namespace silverstar
