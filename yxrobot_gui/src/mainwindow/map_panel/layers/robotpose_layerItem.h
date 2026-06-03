#ifndef ROBOTPOSE_LAYERITEM_H
#define ROBOTPOSE_LAYERITEM_H
#include <iostream>
#include <QObject>
#include <QGraphicsItem>
#include <QPainter>
#include <QCursor>
#include <QGraphicsSceneWheelEvent>
#include <QRectF>
#include "common/common.h"
#include "common/map_coordinate_transformer.h"
#include "mainwindow/map_panel/core/map_layeritem_virtual.h"

namespace silverstar {
namespace map_panel {

class RobotPoseItem : public MapLayerBase
{
    Q_OBJECT
public:
    RobotPoseItem(const QString& id,const QString& name,const int& z,QGraphicsItem* parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;
    QRectF boundingRect() const override;
    void setRobotSize(double width_m, double height_m);

public slots:
    void updateMap(const OccupancyMap& map);
    void updatePose(RobotPose pose);

private:
    void updateRobotPixelSize();
    void drawFootprint(QPainter* painter) const;
    void drawCenterJoint(QPainter* painter) const;
    void drawAxisArrow(QPainter* painter,
                       qreal angle_degrees,
                       const QColor& fill,
                       const QColor& border) const;

    RobotPose m_currRobotPose;
    MapCoordinateTransformer coordinate_transformer_;
    double map_resolution_{0.0};

    double m_robotActualWidth_m = 1.0;  // 机器人实际宽度（米），例如 0.5m
    double m_robotActualHeight_m = 1.0; // 机器人实际长度（米），例如 0.5m
    double m_robotRadius_px{20.0};
    QRectF m_boundingRect{-28.0, -28.0, 56.0, 56.0};
};

}  // namespace map_panel
}  // namespace silverstar

#endif // ROBOTPOSE_LAYERITEM_H
