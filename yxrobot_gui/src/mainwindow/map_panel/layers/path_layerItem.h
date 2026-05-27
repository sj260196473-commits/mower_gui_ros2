#ifndef PATH_LAYERITEM_H
#define PATH_LAYERITEM_H
#include <iostream>
#include "mainwindow/map_panel/core/map_layeritem_virtual.h"
#include "common/common.h"
#include "common/map_coordinate_transformer.h"
#include <QPainter>
#include <QColor>
#include <QGraphicsItem>
#include <QGraphicsSceneWheelEvent>

namespace silverstar {
namespace map_panel {

class PathLayerItem : public MapLayerBase
{
    Q_OBJECT
public:
    PathLayerItem(const QString& id,const QString& name,const int& z,QGraphicsItem* parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;
    QRectF boundingRect() const;

private:
    void computeBoundRect(const Path& path);
    void drawPath(QPainter *painter, const Path& path);
    void rebuildScenePath();

public slots:
    void updateMap(const OccupancyMap& map);
    void UpdatePath(const Path& path);

private:
    QRectF bounding_rect_;
    MapCoordinateTransformer coordinate_transformer_;
    Path current_path_world_;
    Path current_path_scene_;
};

}  // namespace map_panel
}  // namespace silverstar

#endif // PATH_LAYERITEM_H
