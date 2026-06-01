#include "mainwindow/map_panel/view/map_graphicsview.h"
#include "mainwindow/map_panel/core/editable_zone_model.h"
#include "mainwindow/map_panel/core/pnc_task_model.h"
#include "mainwindow/map_panel/layers/editable_zone_layeritem.h"
#include "mainwindow/map_panel/layers/grid_layeritem.h"
#include "mainwindow/map_panel/view/map_overlay_widget.h"
#include <QAction>
#include <QMenu>
#include <QPainter>
#include <algorithm>
#include <cmath>

namespace silverstar {
namespace map_panel {

namespace
{
constexpr double kGridCellPixelLength = 80.0;
constexpr double kMinViewScale = 0.01;
constexpr double kScaleStep = 1.15;
}

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

    layer_runtime_ = std::make_unique<MapLayerRuntime>();
    layer_runtime_->initializeDefaultLayers(m_qGraphicScene);

    // 在图层管理器上的按键选择层
    pnc_task_overlay_widget_ = new MapOverlayWidget(viewport());
    setupPncTaskOverlay();
    overlay_widget_ = new MapOverlayWidget(viewport());
    setupZoneEditorOverlay();
    updateOverlayGeometry();

    this->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    this->setRenderHint(QPainter::Antialiasing);


}

void MapGraphicsView::setCommChannel(VirtualChannel* channel)
{
    if (channel == nullptr) {
        return;
    }

    channel_ = channel;
    layer_runtime_->bindChannel(channel);

    connect(channel, &VirtualChannel::emitUpdateMap,
            this, &MapGraphicsView::updateMap, Qt::QueuedConnection);
}

void MapGraphicsView::focusMapView()
{
    if (current_map_scene_rect_.isNull() || current_map_scene_rect_.isEmpty()) {
        return;
    }

    focusOnRect(current_map_scene_rect_);
    updateGridCellLengthStatus();
    updateOverlayGeometry();
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
    if (layer_runtime_->gridLayer()) {
        layer_runtime_->gridLayer()->setSceneRect(interactionSceneRect);
    }
    if (layer_runtime_->editableZoneLayer()) {
        layer_runtime_->editableZoneLayer()->setSceneRect(interactionSceneRect);
        layer_runtime_->editableZoneLayer()->updateMap(map);
    }

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

    if (handleZoneEditorMousePress(event)) {
        return;
    }
    if (handlePncTaskMousePress(event)) {
        return;
    }

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
    const double maxScale = kGridCellPixelLength;

    double scaleFactor = 1.0;
    if (wheelDelta > 0 && currentScale < maxScale) {
        scaleFactor = std::min(kScaleStep, maxScale / currentScale);
    } else if (wheelDelta < 0 && currentScale > kMinViewScale) {
        scaleFactor = std::max(1.0 / kScaleStep, kMinViewScale / currentScale);
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
    updateOverlayGeometry();
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

void MapGraphicsView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    updateOverlayGeometry();
}

void MapGraphicsView::scrollContentsBy(int dx, int dy)
{
    QGraphicsView::scrollContentsBy(dx, dy);
    updateOverlayGeometry();
}

void MapGraphicsView::showLayerContextMenu(const QPoint& global_pos)
{
    QMenu menu(this);
    QMenu* mapLayerMenu = menu.addMenu("MapLayers");
    QMenu* pathLayerMenu = menu.addMenu("PathLayers");
    QMenu* otherLayerMenu = menu.addMenu("OtherLayers");
    auto& registry = layer_runtime_->registry();

    for (const MapLayerEntry& layer : registry.layers()) {
        if (!layer.item) {
            continue;
        }
        QAction* action = nullptr;
        if(layer.group == "map") {
            action = mapLayerMenu->addAction(layer.name);
        }
        else if(layer.group == "path") {
            action = pathLayerMenu->addAction(layer.name);
        }
        else {
            action = otherLayerMenu->addAction(layer.name);
        }
        action->setCheckable(true);
        action->setChecked(registry.isVisible(layer.id));

        const QString layerId = layer.id;
        connect(action, &QAction::toggled, this, [this, layerId](bool checked) {
            layer_runtime_->registry().setVisible(layerId, checked);
        });
    }

    menu.exec(global_pos);
}

void MapGraphicsView::setupZoneEditorOverlay()
{
    if (!overlay_widget_) {
        return;
    }

    overlay_widget_->addButton("zone.draw.no_entry", "禁区", "绘制禁区");
    overlay_widget_->addButton("zone.draw.virtual_wall", "虚拟墙", "绘制虚拟墙");
    overlay_widget_->addButton("zone.draw.obstacle", "障碍", "绘制障碍物");
    overlay_widget_->addButton("zone.draw.furniture", "家具", "绘制家具区域");
    overlay_widget_->addButton("zone.finish", "完成", "完成当前多边形");
    overlay_widget_->addButton("zone.cancel", "取消", "取消当前绘制");
    overlay_widget_->addButton("zone.delete", "删除", "删除选中的区域");
    overlay_widget_->addButton("zone.clear", "清空", "清空所有编辑区域");
    overlay_widget_->setInfoText("Zone editor ready");

    connect(overlay_widget_, &MapOverlayWidget::buttonClicked,
            this, &MapGraphicsView::handleZoneOverlayButtonClicked);
}

void MapGraphicsView::setupPncTaskOverlay()
{
    if (!pnc_task_overlay_widget_) {
        return;
    }

    pnc_task_overlay_widget_->addButton("pnc.p2p", "点到点", "点击后在地图上选择目标点");
    pnc_task_overlay_widget_->addButton("pnc.coverage", "覆盖", "对当前选中区域发送覆盖任务");
    pnc_task_overlay_widget_->addButton("pnc.along_wall", "沿墙", "对当前选中区域发送沿墙任务");
    pnc_task_overlay_widget_->setInfoText("PNC task ready");

    connect(pnc_task_overlay_widget_, &MapOverlayWidget::buttonClicked,
            this, &MapGraphicsView::handlePncTaskButtonClicked);
}

void MapGraphicsView::handleZoneOverlayButtonClicked(const QString& id)
{
    if (id == QStringLiteral("zone.draw.no_entry")) {
        startDrawingZone(EditableZoneKind::NoEntry);
    } else if (id == QStringLiteral("zone.draw.virtual_wall")) {
        startDrawingZone(EditableZoneKind::VirtualWall);
    } else if (id == QStringLiteral("zone.draw.obstacle")) {
        startDrawingZone(EditableZoneKind::Obstacle);
    } else if (id == QStringLiteral("zone.draw.furniture")) {
        startDrawingZone(EditableZoneKind::Furniture);
    } else if (id == QStringLiteral("zone.finish")) {
        finishDrawingZone();
    } else if (id == QStringLiteral("zone.cancel")) {
        cancelDrawingZone();
    } else if (id == QStringLiteral("zone.delete")) {
        deleteSelectedZone();
    } else if (id == QStringLiteral("zone.clear")) {
        clearEditableZones();
    }
}

void MapGraphicsView::handlePncTaskButtonClicked(const QString& id)
{
    if (id == QStringLiteral("pnc.p2p")) {
        startP2PGoalPicking();
    } else if (id == QStringLiteral("pnc.coverage")) {
        sendSelectedZoneTask(PncTaskType::Coverage);
    } else if (id == QStringLiteral("pnc.along_wall")) {
        sendSelectedZoneTask(PncTaskType::AlongWall);
    }
}

bool MapGraphicsView::handleZoneEditorMousePress(QMouseEvent* event)
{
    auto* zoneLayer = layer_runtime_->editableZoneLayer();
    if (!zoneLayer || !zoneLayer->isVisible()) {
        return false;
    }

    bool hasWorld = false;
    const QPointF worldPoint = viewToWorld(event->pos(), &hasWorld);
    if (!hasWorld) {
        return false;
    }

    if (event->button() == Qt::LeftButton && zoneLayer->isDrawing()) {
        zoneLayer->addWorldPoint(worldPoint);
        event->accept();
        return true;
    }

    if (event->button() == Qt::LeftButton && (event->modifiers() & Qt::ControlModifier)) {
        zoneLayer->selectAtWorldPoint(worldPoint);
        event->accept();
        return true;
    }

    return false;
}

bool MapGraphicsView::handlePncTaskMousePress(QMouseEvent* event)
{
    if (!picking_p2p_goal_) {
        return false;
    }

    if (event->button() != Qt::LeftButton) {
        return false;
    }

    bool hasWorld = false;
    const QPointF worldPoint = viewToWorld(event->pos(), &hasWorld);
    if (!hasWorld) {
        return false;
    }

    sendP2PTask(worldPoint);
    picking_p2p_goal_ = false;
    setCursor(Qt::ArrowCursor);
    event->accept();
    return true;
}

void MapGraphicsView::startP2PGoalPicking()
{
    picking_p2p_goal_ = true;
    if (pnc_task_overlay_widget_) {
        pnc_task_overlay_widget_->setInfoText(QStringLiteral("Click map goal"));
    }
    setCursor(Qt::CrossCursor);
}

void MapGraphicsView::sendP2PTask(const QPointF& worldPoint)
{
    if (!channel_) {
        return;
    }

    channel_->SendPncTask(serializeP2PTaskToJson(worldPoint));
    if (pnc_task_overlay_widget_) {
        pnc_task_overlay_widget_->setInfoText(
            QStringLiteral("P2P sent: %1, %2").arg(worldPoint.x(), 0, 'f', 2).arg(worldPoint.y(), 0, 'f', 2));
    }
}

void MapGraphicsView::sendSelectedZoneTask(PncTaskType type)
{
    auto* zoneLayer = layer_runtime_->editableZoneLayer();
    if (!channel_ || !zoneLayer) {
        return;
    }

    EditableZone zone;
    if (!zoneLayer->selectedZone(&zone)) {
        if (pnc_task_overlay_widget_) {
            pnc_task_overlay_widget_->setInfoText(QStringLiteral("Ctrl+click a zone first"));
        }
        return;
    }

    channel_->SendPncTask(serializeZoneTaskToJson(type, zone));
    if (pnc_task_overlay_widget_) {
        pnc_task_overlay_widget_->setInfoText(QStringLiteral("%1 sent").arg(pncTaskTypeName(type)));
    }
}

void MapGraphicsView::startDrawingZone(EditableZoneKind kind)
{
    auto* zoneLayer = layer_runtime_->editableZoneLayer();
    if (!zoneLayer) {
        return;
    }
    zoneLayer->setVisible(true);
    zoneLayer->beginDrawing(kind);
    if (overlay_widget_) {
        overlay_widget_->setInfoText(QStringLiteral("Drawing %1").arg(zoneKindDisplayName(kind)));
    }
    setCursor(Qt::CrossCursor);
}

void MapGraphicsView::finishDrawingZone()
{
    auto* zoneLayer = layer_runtime_->editableZoneLayer();
    if (!zoneLayer) {
        return;
    }
    if (zoneLayer->finishDrawing()) {
        if (overlay_widget_) {
            overlay_widget_->setInfoText(QStringLiteral("Zone committed"));
        }
        setCursor(Qt::ArrowCursor);
    }
}

void MapGraphicsView::cancelDrawingZone()
{
    auto* zoneLayer = layer_runtime_->editableZoneLayer();
    if (!zoneLayer) {
        return;
    }
    zoneLayer->cancelDrawing();
    if (overlay_widget_) {
        overlay_widget_->setInfoText(QStringLiteral("Drawing cancelled"));
    }
    setCursor(Qt::ArrowCursor);
}

void MapGraphicsView::deleteSelectedZone()
{
    auto* zoneLayer = layer_runtime_->editableZoneLayer();
    if (zoneLayer) {
        if (zoneLayer->removeSelectedZone() && overlay_widget_) {
            overlay_widget_->setInfoText(QStringLiteral("Zone deleted"));
        }
    }
}

void MapGraphicsView::clearEditableZones()
{
    auto* zoneLayer = layer_runtime_->editableZoneLayer();
    if (zoneLayer) {
        zoneLayer->clearZones();
        if (overlay_widget_) {
            overlay_widget_->setInfoText(QStringLiteral("Zones cleared"));
        }
    }
}

QPointF MapGraphicsView::viewToWorld(const QPoint& view_pos, bool* ok) const
{
    if (ok) {
        *ok = false;
    }
    if (!coordinate_transformer_.isValid()) {
        return QPointF();
    }

    const QPointF scenePos = mapToScene(view_pos);
    const Point worldPoint = coordinate_transformer_.sceneToWorld(Point(scenePos.x(), scenePos.y()));
    if (ok) {
        *ok = true;
    }
    return QPointF(worldPoint.x, worldPoint.y);
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
    if (layer_runtime_->gridLayer()) {
        layer_runtime_->gridLayer()->setGridSceneLength(gridSize);
    }
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

    return kGridCellPixelLength / pixelsPerSceneUnit;
}

void MapGraphicsView::updateOverlayGeometry()
{
    if (!overlay_widget_) {
        return;
    }

    constexpr int kOverlayMargin = 12;
    const int width = std::max(0, viewport()->width() - kOverlayMargin * 2);
    const int height = overlay_widget_->height();
    const int x = kOverlayMargin;
    const int y = std::max(kOverlayMargin, viewport()->height() - height - kOverlayMargin);

    overlay_widget_->setGeometry(x, y, width, height);
    overlay_widget_->raise();

    if (pnc_task_overlay_widget_) {
        const int topY = kOverlayMargin;
        pnc_task_overlay_widget_->setGeometry(x, topY, width, pnc_task_overlay_widget_->height());
        pnc_task_overlay_widget_->raise();
    }
}

}  // namespace map_panel
}  // namespace silverstar
