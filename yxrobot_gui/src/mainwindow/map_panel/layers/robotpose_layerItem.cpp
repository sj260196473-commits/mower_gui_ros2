#include "robotpose_layerItem.h"

#include <algorithm>
#include <cmath>

namespace silverstar {
namespace map_panel {

/// 初始化机器人位姿图层元信息和默认绘制尺寸。
RobotPoseItem::RobotPoseItem(const QString& id,const QString& name,const int& z,QGraphicsItem* parent)
    :MapLayerBase(id,name,"localization",parent)
{
    setZValue(z);
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptDrops(true);
    setFlag(ItemAcceptsInputMethod, true);
    moveBy(0, 0);
    updateRobotPixelSize();
}

/// 返回机器人图元当前包围盒。
QRectF RobotPoseItem::boundingRect() const {
    return m_boundingRect;
}

/// 根据当前位姿朝向绘制机器人轮廓和局部坐标轴。
void RobotPoseItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                     QWidget *widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing, true);  //设置反锯齿 反走样
    painter->save();
    painter->rotate(rad2deg(m_currRobotPose.theta));//旋转到机器人坐标theta
    drawFootprint(painter);
    drawAxisArrow(painter,
                  0.0,
                  QColor(235, 18, 18, 235),
                  QColor(155, 0, 0, 245));
    drawAxisArrow(painter,
                  90.0,
                  QColor(0, 220, 38, 235),
                  QColor(0, 125, 22, 245));
    drawCenterJoint(painter);
    painter->restore();
}

/// 设置机器人圆形 footprint 半径，并按当前地图分辨率更新像素尺寸。
void RobotPoseItem::setRobotRadius(double radius_m)
{
    if (radius_m <= 0.0) {
        return;
    }

    m_robotRadius_m = radius_m;
    prepareGeometryChange();
    updateRobotPixelSize();
    update();
}

/// 更新地图坐标转换器和地图分辨率。
void RobotPoseItem::updateMap(const OccupancyMap& map)
{
    coordinate_transformer_.updateMap(map);
    map_resolution_ = map.getRes();
    prepareGeometryChange();
    updateRobotPixelSize();
    update();
}

/// 更新机器人世界位姿，并将位置转换为 scene 坐标。
void RobotPoseItem::updatePose(RobotPose pose)
{

    QPointF pos;
    pos.setX(pose.x);
    pos.setY(pose.y);
    // QPointF scenePose = occMapItem_->mapToScene(pos);//图层坐标转换map->场景坐标
    m_currRobotPose = pose;
    if (!coordinate_transformer_.isValid()) {
        return;
    }
    Point robotWorldPose(pose.x, pose.y);
    Point robotScenePose = coordinate_transformer_.worldToScene(robotWorldPose);
    // std::cout<<"robotScenePose:"<<robotScenePose.x<<","<<robotScenePose.y<<std::endl;
    // std::cout<<"getPose:"<<pose.x<<","<<pose.y<<std::endl;
    setPos(robotScenePose.x, robotScenePose.y);//设置场景坐标系下坐标
    update();
}

/// 根据真实尺寸和地图分辨率计算绘制半径与包围盒。
void RobotPoseItem::updateRobotPixelSize()
{
    if (map_resolution_ > 0.0) {
        m_robotRadius_px = std::max(3.0, m_robotRadius_m / map_resolution_);
    }

    const double padding = 3.0;
    const double halfExtent = m_robotRadius_px * 2.0 + padding;
    m_boundingRect = QRectF(-halfExtent, -halfExtent, halfExtent * 2.0, halfExtent * 2.0);
}

/// 绘制机器人占地轮廓。
void RobotPoseItem::drawFootprint(QPainter* painter) const
{
    const QRectF footprintRect(-m_robotRadius_px,
                               -m_robotRadius_px,
                               m_robotRadius_px * 2.0,
                               m_robotRadius_px * 2.0);

    painter->setPen(QPen(QColor(30, 85, 150, 210), std::max(1.2, m_robotRadius_px * 0.08)));
    painter->setBrush(QColor(46, 144, 235, 38));
    painter->drawEllipse(footprintRect);
}

/// 绘制机器人中心点。
void RobotPoseItem::drawCenterJoint(QPainter* painter) const
{
    const double radius = std::min(1.0, m_robotRadius_px * 0.28);
    const QRectF jointRect(-radius, -radius, radius * 2.0, radius * 2.0);

    painter->setPen(QPen(QColor(0, 38, 190, 240), 0.8));
    painter->setBrush(QColor(0, 82, 255, 245));
    painter->drawEllipse(jointRect);
}

/// 绘制机器人局部坐标轴箭头。
void RobotPoseItem::drawAxisArrow(QPainter* painter,
                                  qreal angle_degrees,
                                  const QColor& fill,
                                  const QColor& border) const
{
    const double shaftWidth = 1.0;
    const double headHalfWidth = 1.0;
    const double headLength = std::max(1.0, m_robotRadius_px * 0.24);
    const double axisTip = m_robotRadius_px * 2.0;
    const double shaftEnd = axisTip - headLength * 0.65;
    const double shaftStart = std::min(1.0, m_robotRadius_px * 0.18);
    const QRectF shaftRect(shaftStart,
                           -shaftWidth / 2.0,
                           shaftEnd - shaftStart,
                           shaftWidth);
    QPolygonF arrowHead;
    arrowHead << QPointF(axisTip, 0.0)
              << QPointF(shaftEnd, -headHalfWidth)
              << QPointF(shaftEnd, headHalfWidth);

    painter->save();
    painter->rotate(angle_degrees);

    QLinearGradient shine(0.0, -shaftWidth / 2.0, 0.0, shaftWidth / 2.0);
    shine.setColorAt(0.0, border);
    shine.setColorAt(0.42, fill.lighter(125));
    shine.setColorAt(0.58, fill);
    shine.setColorAt(1.0, border);

    painter->setPen(QPen(border, 1.0));
    painter->setBrush(shine);
    painter->drawRoundedRect(shaftRect, shaftWidth / 2.0, shaftWidth / 2.0);

    painter->setPen(QPen(border, 1.4));
    painter->setBrush(fill);
    painter->drawPolygon(arrowHead);
    painter->restore();
}

}  // namespace map_panel
}  // namespace silverstar
