#include "robotpose_layerItem.h"

RobotPoseItem::RobotPoseItem(const QString& id,const QString& name,const int& z,QGraphicsItem* parent)
    :MapLayerItemVirtual(id,name,"localization",parent)
{
    setZValue(z);
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptDrops(true);
    setFlag(ItemAcceptsInputMethod, true);
    moveBy(0, 0);

    //加载图片
    m_originalImg.load(":/sweeper.png");
    QMatrix matrix;
    matrix.rotate(90);
    m_originalImg = m_originalImg.transformed(matrix, Qt::SmoothTransformation);
    m_robotImg = m_originalImg;
}

QRectF RobotPoseItem::boundingRect() const {
    qreal w = m_robotImg.width();
    qreal h = m_robotImg.height();
    return QRectF(-w / 2.0, -h / 2.0, w, h);
}

void RobotPoseItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                     QWidget *widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing, true);  //设置反锯齿 反走样
    painter->save();
    painter->rotate(rad2deg(m_currRobotPose.theta));//旋转到机器人坐标theta
    painter->drawPixmap(-m_robotImg.width() / 2, -m_robotImg.height() / 2,
                        m_robotImg);
    painter->restore();
}

void RobotPoseItem::updateMap(const OccupancyMap& map)
{
    coordinate_transformer_.updateMap(map);
    map_resolution_ = map.getRes();

    // 获取地图分辨率 (假设方法名为 resolution() 或 getResolution()，单位 m/pixel)
    double res = map_resolution_;

    if (res > 0.0) {
        // 将 米 转换为 像素
        int pixelWidth = static_cast<int>(m_robotActualWidth_m / res);
        int pixelHeight = static_cast<int>(m_robotActualHeight_m / res);

        // 关键：通知QGraphicsScene当前Item的边界发生了变化，需要重绘
        prepareGeometryChange();

        // 使用原图进行缩放，保证清晰度
        m_robotImg = m_originalImg.scaled(pixelWidth, pixelHeight,
                                          Qt::IgnoreAspectRatio,
                                          Qt::SmoothTransformation);
    }
}

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
