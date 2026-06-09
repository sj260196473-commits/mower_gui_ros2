#ifndef MAP_GRAPHICSVIEW_H
#define MAP_GRAPHICSVIEW_H
#include <QGraphicsView>
#include "mainwindow/map_panel/core/map_layer_runtime.h"
#include "channel/virtual_channel.h"
#include "common/map_coordinate_transformer.h"
#include <QEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QContextMenuEvent>
#include <QScrollBar>
#include <QDebug>
#include <memory>

class QGraphicsLineItem;
class QGraphicsPolygonItem;
class QResizeEvent;

namespace silverstar {
namespace map_panel {

class MapOverlayWidget;
enum class EditableZoneKind;
enum class PncTaskType;

class MapGraphicsView: public QGraphicsView
{
    Q_OBJECT
public:
    /// 构造地图视图，初始化 scene、默认图层和悬浮工具条。
    MapGraphicsView(QWidget* parent = nullptr);

    /// 绑定通信通道，并连接地图更新信号。
    void setCommChannel(VirtualChannel* channel);

public slots:
    /// 将视图重新聚焦到当前地图范围。
    void focusMapView();

protected:
    // 重写鼠标与滚轮事件
    /// 处理区域编辑、P2P 选择和默认拖拽起点。
    void mousePressEvent(QMouseEvent *event) override;

    /// 处理地图拖拽、P2P 箭头预览和坐标状态更新。
    void mouseMoveEvent(QMouseEvent *event) override;

    /// 处理 P2P 释放发送和默认拖拽结束。
    void mouseReleaseEvent(QMouseEvent *event) override;

    /// 处理以鼠标位置为中心的地图缩放。
    void wheelEvent(QWheelEvent *event) override;

    /// 鼠标离开视图时通知状态栏清空坐标。
    void leaveEvent(QEvent *event) override;

    /// 显示图层显隐右键菜单。
    void contextMenuEvent(QContextMenuEvent *event) override;

    /// 视图尺寸变化时重新布局悬浮工具条。
    void resizeEvent(QResizeEvent *event) override;

    /// 滚动内容后重新布局悬浮工具条。
    void scrollContentsBy(int dx, int dy) override;

signals:
    /// 鼠标坐标变化信号，同时携带 scene 坐标和可选 world 坐标。
    void mousePositionChanged(const QPointF& scene_pos, const QPointF& world_pos, bool has_world);

    /// 鼠标离开地图视图信号。
    void mouseLeftScene();

    /// 网格单元世界长度变化信号。
    void gridCellLengthChanged(double length_m);

private:
    /// 根据 registry 构建图层显隐菜单。
    void showLayerContextMenu(const QPoint& global_pos);

    /// 初始化区域编辑悬浮按钮条。
    void setupZoneEditorOverlay();

    /// 初始化 PNC 任务悬浮按钮条。
    void setupPncTaskOverlay();

    /// 分发区域编辑按钮点击事件。
    void handleZoneOverlayButtonClicked(const QString& id);

    /// 分发 PNC 按钮点击事件。
    void handlePncTaskButtonClicked(const QString& id);

    /// 处理区域编辑模式下的鼠标按下。
    bool handleZoneEditorMousePress(QMouseEvent* event);

    /// 处理 P2P 模式下的鼠标按下，记录目标起点。
    bool handlePncTaskMousePress(QMouseEvent* event);

    /// 处理 P2P 模式下的鼠标释放，计算朝向并发送任务。
    bool handlePncTaskMouseRelease(QMouseEvent* event);

    /// 进入 P2P 目标选择模式。
    void startP2PGoalPicking();

    /// 发送 P2P 任务，press 为目标位置，release 用于计算 yaw。
    void sendP2PTask(const QPointF& pressWorldPoint, const QPointF& releaseWorldPoint);

    /// 更新 P2P 拖拽方向箭头预览。
    void updateP2PGoalPreview(const QPointF& pressWorldPoint, const QPointF& releaseWorldPoint);

    /// 清理 P2P 方向箭头预览图元。
    void clearP2PGoalPreview();

    /// 将世界坐标转换为 scene 坐标。
    QPointF worldToScenePoint(const QPointF& worldPoint) const;

    /// 基于当前选中区域发送覆盖或沿墙任务。
    void sendSelectedZoneTask(PncTaskType type);

    /// 进入指定类型区域绘制模式。
    void startDrawingZone(EditableZoneKind kind);

    /// 完成当前区域绘制。
    void finishDrawingZone();

    /// 取消当前区域绘制。
    void cancelDrawingZone();

    /// 删除当前选中区域。
    void deleteSelectedZone();

    /// 清空所有编辑区域。
    void clearEditableZones();

    /// 将视图坐标转换为世界坐标，并通过 ok 返回是否成功。
    QPointF viewToWorld(const QPoint& view_pos, bool* ok = nullptr) const;

    /// 将视图适配到指定 scene 矩形。
    void focusOnRect(const QRectF& targetRect);

    /// 计算并发出当前鼠标坐标状态。
    void emitMousePosition(const QPoint& view_pos);

    /// 根据当前缩放更新网格单元长度状态。
    void updateGridCellLengthStatus();

    /// 返回一个网格单元在 scene 坐标中的长度。
    double gridCellSceneLength() const;

    /// 根据 viewport 尺寸重新摆放悬浮工具条。
    void updateOverlayGeometry();

private slots:
    /// 接收地图更新并刷新 scene 范围、图层和坐标转换器。
    void updateMap(const OccupancyMap& map);

private:
    QGraphicsScene* m_qGraphicScene;
    std::unique_ptr<MapLayerRuntime> layer_runtime_;
    MapOverlayWidget* overlay_widget_{nullptr};
    MapOverlayWidget* pnc_task_overlay_widget_{nullptr};
    VirtualChannel* channel_{nullptr};
    MapCoordinateTransformer coordinate_transformer_;
    QRectF current_map_scene_rect_;


    QPoint m_lastMousePos;  // 记录上一次鼠标的位置
    bool m_isDragging = false; // 是否正在拖拽的标志位
    bool picking_p2p_goal_{false};
    bool has_p2p_goal_press_{false};
    QPointF p2p_goal_press_world_;
    QGraphicsLineItem* p2p_goal_arrow_line_{nullptr};
    QGraphicsPolygonItem* p2p_goal_arrow_head_{nullptr};
    bool has_initial_map_focus_{false};
    double current_grid_cell_length_m_{0.0};
};

}  // namespace map_panel
}  // namespace silverstar

#endif // MAP_GRAPHICSVIEW_H
