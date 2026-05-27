#ifndef ROBOTPOSE_LAYERITEM_H
#define ROBOTPOSE_LAYERITEM_H
#include <iostream>
#include <QObject>
#include <QGraphicsItem>
#include <QPainter>
#include <QCursor>
#include <QGraphicsSceneWheelEvent>
#include "common/common.h"
#include "common/map_coordinate_transformer.h"
#include "mainwindow/map_panel/map_layeritem_virtual.h"

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

public slots:
    void updateMap(const OccupancyMap& map);
    void updatePose(RobotPose pose);

private:
    QPixmap m_robotImg;
    RobotPose m_currRobotPose;
    MapCoordinateTransformer coordinate_transformer_;
    double map_resolution_{0.0};

    double m_robotActualWidth_m = 1.0;  // 机器人实际宽度（米），例如 0.5m
    double m_robotActualHeight_m = 1.0; // 机器人实际长度（米），例如 0.5m
    QPixmap m_originalImg; // 缓存原始图片，防止多次缩放导致失真
};

}  // namespace map_panel
}  // namespace silverstar

#endif // ROBOTPOSE_LAYERITEM_H
