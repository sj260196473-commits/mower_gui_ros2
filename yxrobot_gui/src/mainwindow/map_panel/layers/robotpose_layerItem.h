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
    /// 构造机器人位姿图层并设置 z 值。
    RobotPoseItem(const QString& id,const QString& name,const int& z,QGraphicsItem* parent = nullptr);

    /// 绘制机器人轮廓、中心点和坐标轴箭头。
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

    /// 返回机器人图元包围盒。
    QRectF boundingRect() const override;

    /// 设置机器人圆形 footprint 半径，单位为米。
    void setRobotRadius(double radius_m);

public slots:
    /// 更新地图转换参数和机器人像素尺寸。
    void updateMap(const OccupancyMap& map);

    /// 更新当前机器人世界位姿。
    void updatePose(RobotPose pose);

private:
    /// 根据地图分辨率把机器人半径转换为像素尺寸。
    void updateRobotPixelSize();

    /// 绘制机器人圆形轮廓。
    void drawFootprint(QPainter* painter) const;

    /// 绘制机器人中心关节点。
    void drawCenterJoint(QPainter* painter) const;

    /// 绘制机器人局部坐标轴箭头。
    void drawAxisArrow(QPainter* painter,
                       qreal angle_degrees,
                       const QColor& fill,
                       const QColor& border) const;

    RobotPose m_currRobotPose;
    MapCoordinateTransformer coordinate_transformer_;
    double map_resolution_{0.0};

    double m_robotRadius_m{0.1775};
    double m_robotRadius_px{20.0};
    QRectF m_boundingRect{-28.0, -28.0, 56.0, 56.0};
};

}  // namespace map_panel
}  // namespace silverstar

#endif // ROBOTPOSE_LAYERITEM_H
