#include "mainwindow/map_panel/view/map_graphicsview.h"
#include "mainwindow/map_panel/core/editable_zone_model.h"
#include "mainwindow/map_panel/core/pnc_task_model.h"
#include "mainwindow/map_panel/layers/editable_zone_layeritem.h"
#include "mainwindow/map_panel/layers/grid_layeritem.h"
#include "mainwindow/map_panel/view/map_overlay_widget.h"
#include <QAction>
#include <QBrush>
#include <QGraphicsLineItem>
#include <QGraphicsPolygonItem>
#include <QMenu>
#include <QPainter>
#include <QPen>
#include <algorithm>
#include <cmath>

namespace silverstar {
namespace map_panel {

namespace
{
constexpr double kGridCellPixelLength = 80.0;
constexpr double kMinViewScale = 0.01;
constexpr double kScaleStep = 1.15;
constexpr qreal kP2PArrowHeadLengthPx = 18.0;
constexpr qreal kP2PArrowHeadWidthPx = 11.0;
constexpr qreal kP2PArrowMinSceneLength = 1e-6;
}

/// 构造地图视图，创建 scene、默认图层和上下两个悬浮工具条。
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

/// 绑定通信通道，并订阅地图更新信号。
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

/// 将视图缩放和平移到当前地图矩形。
void MapGraphicsView::focusMapView()
{
    if (current_map_scene_rect_.isNull() || current_map_scene_rect_.isEmpty()) {
        return;
    }

    focusOnRect(current_map_scene_rect_);
    updateGridCellLengthStatus();
    updateOverlayGeometry();
}

/// 更新地图相关状态，包括坐标转换、scene 范围和图层地图缓存。
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

/// 对目标矩形增加边距后适配到视图。
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
/// 鼠标按下入口，优先交给区域编辑和 P2P 交互处理。
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
/// 鼠标移动入口，更新坐标显示、P2P 箭头或地图拖拽。
void MapGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    emitMousePosition(event->pos());

    if (picking_p2p_goal_ && has_p2p_goal_press_) {
        bool hasWorld = false;
        const QPointF releaseWorldPoint = viewToWorld(event->pos(), &hasWorld);
        if (hasWorld) {
            updateP2PGoalPreview(p2p_goal_press_world_, releaseWorldPoint);
        }
        event->accept();
        return;
    }

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
/// 鼠标释放入口，完成 P2P 任务或结束地图拖拽。
void MapGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if (handlePncTaskMouseRelease(event)) {
        return;
    }

    if (event->button() == Qt::LeftButton || event->button() == Qt::MiddleButton) {
        m_isDragging = false;
        setCursor(Qt::ArrowCursor); // 恢复正常鼠标图标
        event->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

// 接管滚轮缩放（比内置的更平滑，且可以限制缩放比例）
/// 滚轮缩放地图，并保持鼠标所在 scene 点缩放前后位置稳定。
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

/// 鼠标离开视图时通知外部清理坐标状态。
void MapGraphicsView::leaveEvent(QEvent *event)
{
    emit mouseLeftScene();
    QGraphicsView::leaveEvent(event);
}

/// 显示图层显隐右键菜单。
void MapGraphicsView::contextMenuEvent(QContextMenuEvent *event)
{
    showLayerContextMenu(event->globalPos());
    event->accept();
}

/// 尺寸变化后重新布局悬浮工具条。
void MapGraphicsView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    updateOverlayGeometry();
}

/// 滚动条移动后重新布局悬浮工具条。
void MapGraphicsView::scrollContentsBy(int dx, int dy)
{
    QGraphicsView::scrollContentsBy(dx, dy);
    updateOverlayGeometry();
}

/// 根据 registry 中的图层信息生成右键菜单。
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

/// 创建区域编辑按钮并连接按钮点击信号。
void MapGraphicsView::setupZoneEditorOverlay()
{
    if (!overlay_widget_) {
        return;
    }
    overlay_widget_->addButton("zone.draw.clean_area", "清扫区", "绘制清扫区域");
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

/// 创建 PNC 任务按钮并连接按钮点击信号。
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

/// 根据区域编辑按钮 id 调用对应编辑命令。
void MapGraphicsView::handleZoneOverlayButtonClicked(const QString& id)
{
    if (id == QStringLiteral("zone.draw.no_entry")) {
        startDrawingZone(EditableZoneKind::NoEntry);
    } else if (id == QStringLiteral("zone.draw.virtual_wall")) {
        startDrawingZone(EditableZoneKind::VirtualWall);
    } else if (id == QStringLiteral("zone.draw.clean_area")) {
        startDrawingZone(EditableZoneKind::CleanArea);
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

/// 根据 PNC 按钮 id 调用点到点或区域任务命令。
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

/// 在区域编辑模式下处理加点和 Ctrl 选择区域。
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

/// 在 P2P 模式下记录鼠标按下世界坐标作为目标点。
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

    p2p_goal_press_world_ = worldPoint;
    has_p2p_goal_press_ = true;
    updateP2PGoalPreview(p2p_goal_press_world_, worldPoint);
    if (pnc_task_overlay_widget_) {
        pnc_task_overlay_widget_->setInfoText(QStringLiteral("Drag direction and release"));
    }
    event->accept();
    return true;
}

/// 在 P2P 模式下根据释放点计算朝向并发送任务。
bool MapGraphicsView::handlePncTaskMouseRelease(QMouseEvent* event)
{
    if (!picking_p2p_goal_ || !has_p2p_goal_press_) {
        return false;
    }

    if (event->button() != Qt::LeftButton) {
        return false;
    }

    bool hasWorld = false;
    const QPointF releaseWorldPoint = viewToWorld(event->pos(), &hasWorld);
    if (!hasWorld) {
        return false;
    }

    const QPointF delta = releaseWorldPoint - p2p_goal_press_world_;
    if (std::hypot(delta.x(), delta.y()) < 1e-6) {
        has_p2p_goal_press_ = false;
        clearP2PGoalPreview();
        if (pnc_task_overlay_widget_) {
            pnc_task_overlay_widget_->setInfoText(QStringLiteral("Drag to set direction"));
        }
        event->accept();
        return true;
    }

    sendP2PTask(p2p_goal_press_world_, releaseWorldPoint);
    picking_p2p_goal_ = false;
    has_p2p_goal_press_ = false;
    clearP2PGoalPreview();
    setCursor(Qt::ArrowCursor);
    event->accept();
    return true;
}

/// 进入 P2P 目标选择模式，并取消未完成的区域绘制。
void MapGraphicsView::startP2PGoalPicking()
{
    if (auto* zoneLayer = layer_runtime_->editableZoneLayer()) {
        zoneLayer->cancelDrawing();
    }
    picking_p2p_goal_ = true;
    has_p2p_goal_press_ = false;
    clearP2PGoalPreview();
    if (pnc_task_overlay_widget_) {
        pnc_task_overlay_widget_->setInfoText(QStringLiteral("Press goal and drag direction"));
    }
    setCursor(Qt::CrossCursor);
}

/// 发送 P2P 任务并在工具条显示目标位姿。
void MapGraphicsView::sendP2PTask(const QPointF& pressWorldPoint, const QPointF& releaseWorldPoint)
{
    if (!channel_) {
        return;
    }

    channel_->SendPncTask(makeP2PTask(pressWorldPoint, releaseWorldPoint));
    if (pnc_task_overlay_widget_) {
        const double yaw = std::atan2(releaseWorldPoint.y() - pressWorldPoint.y(),
                                      releaseWorldPoint.x() - pressWorldPoint.x());
        pnc_task_overlay_widget_->setInfoText(
            QStringLiteral("P2P sent: %1, %2, %3")
                .arg(pressWorldPoint.x(), 0, 'f', 2)
                .arg(pressWorldPoint.y(), 0, 'f', 2)
                .arg(yaw, 0, 'f', 2));
    }
}

/// 创建或更新 P2P 拖拽方向箭头。
void MapGraphicsView::updateP2PGoalPreview(const QPointF& pressWorldPoint, const QPointF& releaseWorldPoint)
{
    const QPointF startScene = worldToScenePoint(pressWorldPoint);
    const QPointF endScene = worldToScenePoint(releaseWorldPoint);
    const QPointF vector = endScene - startScene;
    const qreal length = std::hypot(vector.x(), vector.y());

    if (!p2p_goal_arrow_line_) {
        p2p_goal_arrow_line_ = m_qGraphicScene->addLine(QLineF(startScene, endScene));
        p2p_goal_arrow_line_->setZValue(1000.0);
    }
    if (!p2p_goal_arrow_head_) {
        p2p_goal_arrow_head_ = m_qGraphicScene->addPolygon(QPolygonF());
        p2p_goal_arrow_head_->setZValue(1001.0);
    }

    QPen arrowPen(QColor(20, 180, 255, 230));
    arrowPen.setCosmetic(true);
    arrowPen.setWidth(3);
    arrowPen.setCapStyle(Qt::RoundCap);
    p2p_goal_arrow_line_->setPen(arrowPen);
    p2p_goal_arrow_line_->setLine(QLineF(startScene, endScene));

    p2p_goal_arrow_head_->setPen(Qt::NoPen);
    p2p_goal_arrow_head_->setBrush(QColor(20, 180, 255, 210));
    if (length <= kP2PArrowMinSceneLength) {
        p2p_goal_arrow_head_->setPolygon(QPolygonF());
        return;
    }

    const qreal pixelsPerSceneUnit = std::max<qreal>(0.001, std::abs(transform().m11()));
    const qreal headLength = kP2PArrowHeadLengthPx / pixelsPerSceneUnit;
    const qreal headWidth = kP2PArrowHeadWidthPx / pixelsPerSceneUnit;
    const QPointF direction(vector.x() / length, vector.y() / length);
    const QPointF normal(-direction.y(), direction.x());
    const QPointF base = endScene - direction * headLength;

    QPolygonF arrowHead;
    arrowHead << endScene
              << base + normal * (headWidth * 0.5)
              << base - normal * (headWidth * 0.5);
    p2p_goal_arrow_head_->setPolygon(arrowHead);
}

/// 删除 P2P 箭头图元，释放 scene item。
void MapGraphicsView::clearP2PGoalPreview()
{
    if (p2p_goal_arrow_line_) {
        m_qGraphicScene->removeItem(p2p_goal_arrow_line_);
        delete p2p_goal_arrow_line_;
        p2p_goal_arrow_line_ = nullptr;
    }
    if (p2p_goal_arrow_head_) {
        m_qGraphicScene->removeItem(p2p_goal_arrow_head_);
        delete p2p_goal_arrow_head_;
        p2p_goal_arrow_head_ = nullptr;
    }
}

/// 使用当前地图转换器把世界点转换为 scene 点。
QPointF MapGraphicsView::worldToScenePoint(const QPointF& worldPoint) const
{
    if (!coordinate_transformer_.isValid()) {
        return worldPoint;
    }

    const Point scenePoint = coordinate_transformer_.worldToScene(Point(worldPoint.x(), worldPoint.y()));
    return QPointF(scenePoint.x, scenePoint.y);
}

/// 使用当前选中区域发送覆盖或沿墙任务。
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

    channel_->SendPncTask(makeZoneTask(type, zone));
    if (pnc_task_overlay_widget_) {
        pnc_task_overlay_widget_->setInfoText(QStringLiteral("%1 sent").arg(pncTaskTypeName(type)));
    }
}

/// 开始绘制指定类型区域，并关闭 P2P 选择模式。
void MapGraphicsView::startDrawingZone(EditableZoneKind kind)
{
    auto* zoneLayer = layer_runtime_->editableZoneLayer();
    if (!zoneLayer) {
        return;
    }
    picking_p2p_goal_ = false;
    has_p2p_goal_press_ = false;
    clearP2PGoalPreview();
    zoneLayer->setVisible(true);
    zoneLayer->beginDrawing(kind);
    if (overlay_widget_) {
        overlay_widget_->setInfoText(QStringLiteral("Drawing %1").arg(zoneKindDisplayName(kind)));
    }
    setCursor(Qt::CrossCursor);
}

/// 完成当前区域绘制，成功后恢复普通光标。
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

/// 取消当前区域绘制，恢复普通光标。
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

/// 删除编辑层当前选中的区域。
void MapGraphicsView::deleteSelectedZone()
{
    auto* zoneLayer = layer_runtime_->editableZoneLayer();
    if (zoneLayer) {
        if (zoneLayer->removeSelectedZone() && overlay_widget_) {
            overlay_widget_->setInfoText(QStringLiteral("Zone deleted"));
        }
    }
}

/// 清空编辑层所有区域。
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

/// 将 viewport 坐标转换为世界坐标。
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

/// 计算鼠标 scene/world 坐标并发出状态信号。
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

/// 根据当前缩放计算网格代表的世界长度，并通知图层/状态栏。
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

/// 计算固定屏幕像素长度对应的 scene 长度。
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

/// 把区域编辑工具条放在底部，PNC 工具条放在顶部。
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
