#ifndef MAP_GRAPHICSVIEW_H
#define MAP_GRAPHICSVIEW_H
#include <QGraphicsView>
#include "mainwindow/map_panel/occmap_layerItem.h"
#include "mainwindow/map_panel/costmap_layerItem.h"
#include "mainwindow/map_panel/robotpose_layerItem.h"
#include "mainwindow/map_panel/laser_layeritem.h"
#include "mainwindow/map_panel/path_layerItem.h"
#include "channel/virtual_channel.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QDebug>

class MapGraphicsView: public QGraphicsView
{
    Q_OBJECT
public:
    MapGraphicsView(QWidget* parent = nullptr);
    void setCommChannel(VirtualChannel* channel);

protected:
    // 重写鼠标与滚轮事件
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void focusOnRect(const QRectF& targetRect);

private slots:
    void updateMap(const OccupancyMap& map);

private:
    QGraphicsScene* m_qGraphicScene;
    OccMapItem* m_occMapItem;
    CostMapItem* m_globalCostMapItem;
    RobotPoseItem* m_robotPoseItem;
    LaserItem* m_laserScanItem;
    PathLayerItem* m_globalPathItem;


    QPoint m_lastMousePos;  // 记录上一次鼠标的位置
    bool m_isDragging = false; // 是否正在拖拽的标志位
};

#endif // MAP_GRAPHICSVIEW_H
