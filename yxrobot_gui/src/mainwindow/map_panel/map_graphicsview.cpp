#include "mainwindow/map_panel/map_graphicsview.h"
#include <QAction>
#include <QMenu>
#include <QPainter>
#include <algorithm>
#include <cmath>

MapGraphicsView::MapGraphicsView(QWidget* parent) :
    QGraphicsView(parent)
{
    setMouseTracking(true);
    viewport()->setMouseTracking(true);

    //初始化地图场景类
    m_qGraphicScene = new QGraphicsScene();
    this->scale(1,-1);//view沿x轴对称翻转，对其世界坐标系
    this->setScene(m_qGraphicScene);
    this->setBackgroundBrush(QColor(48, 48, 48));

    //创建默认的地图显示图层
    auto* factory = MapDisplayFactory::Instance();
    for (MapLayerItemVirtual* layer : factory->createDefaultDisplays()) {
        addLayerToScene(layer);
    }

    m_occMapItem = dynamic_cast<OccMapItem*>(layer_registry_.layer("map.occMap"));
    m_globalCostMapItem = dynamic_cast<CostMapItem*>(layer_registry_.layer("map.globalCostMap"));
    m_gridItem = dynamic_cast<GridLayerItem*>(layer_registry_.layer("grid.grid"));
    m_robotPoseItem = dynamic_cast<RobotPoseItem*>(layer_registry_.layer("localization.robot"));
    m_laserScanItem = dynamic_cast<LaserItem*>(layer_registry_.layer("scan.laser"));
    m_globalPathItem = dynamic_cast<PathLayerItem*>(layer_registry_.layer("plan.globalPath"));

    this->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    this->setRenderHint(QPainter::Antialiasing);


}

void MapGraphicsView::addLayerToScene(MapLayerItemVirtual* layer)
{
    if (!layer) {
        return;
    }

    m_qGraphicScene->addItem(layer);
    layer_registry_.addLayer(layer);
}

void MapGraphicsView::setCommChannel(VirtualChannel* channel)
{
    if (channel == nullptr) {
        return;
    }

    connect(channel, &VirtualChannel::emitUpdateMap,
            m_occMapItem, &OccMapItem::updateMap, Qt::QueuedConnection);
    connect(channel, &VirtualChannel::emitUpdateMap,
            m_robotPoseItem, &RobotPoseItem::updateMap, Qt::QueuedConnection);
    connect(channel, &VirtualChannel::emitUpdateMap,
            m_laserScanItem, &LaserItem::updateMap, Qt::QueuedConnection);
    connect(channel, &VirtualChannel::emitUpdateMap,
            m_globalPathItem, &PathLayerItem::updateMap, Qt::QueuedConnection);
    connect(channel, &VirtualChannel::emitUpdateMap,
            this, &MapGraphicsView::updateMap, Qt::QueuedConnection);
    connect(channel, &VirtualChannel::emitUpdateGlobalCostMap,
            m_globalCostMapItem, &CostMapItem::updateMap, Qt::QueuedConnection);
    connect(channel, &VirtualChannel::emitUpdateRobotPose,
            m_robotPoseItem, &RobotPoseItem::updatePose, Qt::QueuedConnection);
    connect(channel, &VirtualChannel::emitUpdateLaserScan,
            m_laserScanItem, &LaserItem::UpdateLaserData, Qt::QueuedConnection);
    connect(channel, &VirtualChannel::emitUpdatePath,
            m_globalPathItem, &PathLayerItem::UpdatePath, Qt::QueuedConnection);
}

void MapGraphicsView::focusMapView()
{
    if (current_map_scene_rect_.isNull() || current_map_scene_rect_.isEmpty()) {
        return;
    }

    focusOnRect(current_map_scene_rect_);
    updateGridCellLengthStatus();
}

void MapGraphicsView::updateMap(const OccupancyMap& map)
{
    if(map.isNULL()) return;
    coordinate_transformer_.updateMap(map);

    current_map_scene_rect_ = QRectF(0, 0, map.width(), map.height());
    const qreal margin = std::max(current_map_scene_rect_.width(), current_map_scene_rect_.height()) * 5.0;
    const QRectF interactionSceneRect = current_map_scene_rect_.adjusted(
        -margin,
        -margin,
        margin,
        margin);

    m_qGraphicScene->setSceneRect(interactionSceneRect);
    m_gridItem->setSceneRect(interactionSceneRect);

    if (!has_initial_map_focus_) {
        focusMapView();
        has_initial_map_focus_ = true;
    }

    updateGridCellLengthStatus();
}

void MapGraphicsView::focusOnRect(const QRectF& targetRect)
{
    if(targetRect.isNull() || targetRect.isEmpty()) {
        return;
    }

    // 1. 创建基础矩形并计算 Padding (留出 5% 空白边缘)
    double paddingX = targetRect.width() * 0.05;
    double paddingY = targetRect.height() * 0.05;

    // 2. 扩展矩形
    QRectF paddedRect = targetRect.adjusted(-paddingX, -paddingY, paddingX, paddingY);

    // 3. 视角自适应 (保持比例不变)
    this->fitInView(paddedRect, Qt::KeepAspectRatio);
}

// 鼠标按下：记录起始点，开启拖拽状态
void MapGraphicsView::mousePressEvent(QMouseEvent *event)
{
    emitMousePosition(event->pos());

    if (event->button() == Qt::LeftButton || event->button() == Qt::MiddleButton) {
        m_lastMousePos = event->pos();
        m_isDragging = true;
        setCursor(Qt::ClosedHandCursor); // 变成抓取的“小手”图标
        event->accept();
        return;
    }
    // 其他按键交给父类处理
    QGraphicsView::mousePressEvent(event);
}

// 鼠标移动：计算差值，移动滚动条
void MapGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    emitMousePosition(event->pos());

    if (m_isDragging) {
        // 计算鼠标移动的像素差值
        QPoint delta = event->pos() - m_lastMousePos;
        m_lastMousePos = event->pos();

        // 核心：反向操作滚动条来实现画布的平移
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());

        event->accept();
        return;
    }
    QGraphicsView::mouseMoveEvent(event);
}

// 鼠标释放：恢复状态
void MapGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton || event->button() == Qt::MiddleButton) {
        m_isDragging = false;
        setCursor(Qt::ArrowCursor); // 恢复正常鼠标图标
        event->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

// 接管滚轮缩放（比内置的更平滑，且可以限制缩放比例）
void MapGraphicsView::wheelEvent(QWheelEvent *event)
{
    const int wheelDelta = event->angleDelta().y();
    if (wheelDelta == 0) {
        event->ignore();
        return;
    }

    const double currentScale = transform().m11();
    const double scaleStep = 1.15;
    const double minScale = 0.01;
    const double maxScale = 20.0;

    double scaleFactor = 1.0;
    if (wheelDelta > 0 && currentScale < maxScale) {
        scaleFactor = scaleStep;
    } else if (wheelDelta < 0 && currentScale > minScale) {
        scaleFactor = 1.0 / scaleStep;
    }

    if (scaleFactor == 1.0) {
        event->accept();
        return;
    }

    const QPoint mousePos = event->pos();
    const QPointF scenePosBeforeScale = mapToScene(mousePos);
    scale(scaleFactor, scaleFactor);
    const QPoint viewPosAfterScale = mapFromScene(scenePosBeforeScale);
    const QPoint viewDelta = viewPosAfterScale - mousePos;

    horizontalScrollBar()->setValue(horizontalScrollBar()->value() + viewDelta.x());
    verticalScrollBar()->setValue(verticalScrollBar()->value() + viewDelta.y());

    updateGridCellLengthStatus();
    emitMousePosition(mousePos);
    event->accept();
}

void MapGraphicsView::leaveEvent(QEvent *event)
{
    emit mouseLeftScene();
    QGraphicsView::leaveEvent(event);
}

void MapGraphicsView::contextMenuEvent(QContextMenuEvent *event)
{
    showLayerContextMenu(event->globalPos());
    event->accept();
}

void MapGraphicsView::showLayerContextMenu(const QPoint& global_pos)
{
    QMenu menu(this);
    QMenu* layerMenu = menu.addMenu("Layers");

    for (const MapLayerEntry& layer : layer_registry_.layers()) {
        if (!layer.item) {
            continue;
        }

        QAction* action = layerMenu->addAction(layer.name);
        action->setCheckable(true);
        action->setChecked(layer_registry_.isVisible(layer.id));

        const QString layerId = layer.id;
        connect(action, &QAction::toggled, this, [this, layerId](bool checked) {
            layer_registry_.setVisible(layerId, checked);
        });
    }

    menu.exec(global_pos);
}

void MapGraphicsView::emitMousePosition(const QPoint& view_pos)
{
    const QPointF scene_pos = mapToScene(view_pos);
    QPointF world_pos;
    bool has_world = false;

    if (coordinate_transformer_.isValid()) {
        const Point scene_point(scene_pos.x(), scene_pos.y());
        const Point world_point = coordinate_transformer_.sceneToWorld(scene_point);
        world_pos = QPointF(world_point.x, world_point.y);
        has_world = true;
    }

    emit mousePositionChanged(scene_pos, world_pos, has_world);
}

void MapGraphicsView::updateGridCellLengthStatus()
{
    if (!coordinate_transformer_.isValid()) {
        return;
    }

    const double gridSize = gridCellSceneLength();
    const Point worldStart = coordinate_transformer_.sceneToWorld(Point(0.0, 0.0));
    const Point worldEnd = coordinate_transformer_.sceneToWorld(Point(gridSize, 0.0));
    const double length = std::abs(worldEnd.x - worldStart.x);

    if (std::abs(length - current_grid_cell_length_m_) < 1e-9) {
        return;
    }

    current_grid_cell_length_m_ = length;
    m_gridItem->setGridSceneLength(gridSize);
    emit gridCellLengthChanged(current_grid_cell_length_m_);
}

double MapGraphicsView::gridCellSceneLength() const
{
    if (!coordinate_transformer_.isValid()) {
        return 0.0;
    }

    const double pixelsPerSceneUnit = std::abs(transform().m11());
    if (pixelsPerSceneUnit <= 0.0) {
        return 0.0;
    }

    const double targetPixelLength = 80.0;
    return targetPixelLength / pixelsPerSceneUnit;
}
