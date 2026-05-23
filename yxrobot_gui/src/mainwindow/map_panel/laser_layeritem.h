#ifndef LASER_LAYERITEM_H
#define LASER_LAYERITEM_H
#include "mainwindow/map_panel/map_layeritem_virtual.h"
#include "common/common.h"
#include "common/map_coordinate_transformer.h"
#include <QPainter>
#include <QColor>
#include <QGraphicsItem>
#include <QGraphicsSceneWheelEvent>
#include <map>


class LaserItem : public MapLayerItemVirtual
{
public:
    LaserItem(const QString& id,const QString& name,const int& z,QGraphicsItem* parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;
    QRectF boundingRect() const;

    void updateMap(const OccupancyMap& map);
    void UpdateLaserData(const LaserScan& scan);

private:
    QColor Id2Color(int id);
    void drawLaser(QPainter *painter, int id, const std::vector<Point>& data);
    void computeBoundRect(const std::map<int, std::vector<Point>> &laser_scan);
    std::vector<Point> convertWorldPointsToScene(const std::vector<Point>& world_points) const;
    void rebuildSceneLaserData();

private:
    MapCoordinateTransformer coordinate_transformer_;
    std::map<int, QColor> location_to_color_;
    std::map<int, std::vector<Point>> laser_data_world_;
    std::map<int, std::vector<Point>> laser_data_scene_;
    QRectF bounding_rect_;
};

#endif // LASER_LAYERITEM_H
