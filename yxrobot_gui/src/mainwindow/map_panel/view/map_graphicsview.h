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

class QResizeEvent;

namespace silverstar {
namespace map_panel {

class MapOverlayWidget;

class MapGraphicsView: public QGraphicsView
{
    Q_OBJECT
public:
    MapGraphicsView(QWidget* parent = nullptr);
    void setCommChannel(VirtualChannel* channel);

public slots:
    void focusMapView();

protected:
    // 重写鼠标与滚轮事件
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void scrollContentsBy(int dx, int dy) override;

signals:
    void mousePositionChanged(const QPointF& scene_pos, const QPointF& world_pos, bool has_world);
    void mouseLeftScene();
    void gridCellLengthChanged(double length_m);

private:
    void showLayerContextMenu(const QPoint& global_pos);
    void focusOnRect(const QRectF& targetRect);
    void emitMousePosition(const QPoint& view_pos);
    void updateGridCellLengthStatus();
    double gridCellSceneLength() const;
    void updateOverlayGeometry();

private slots:
    void updateMap(const OccupancyMap& map);

private:
    QGraphicsScene* m_qGraphicScene;
    std::unique_ptr<MapLayerRuntime> layer_runtime_;
    MapOverlayWidget* overlay_widget_{nullptr};
    MapCoordinateTransformer coordinate_transformer_;
    QRectF current_map_scene_rect_;


    QPoint m_lastMousePos;  // 记录上一次鼠标的位置
    bool m_isDragging = false; // 是否正在拖拽的标志位
    bool has_initial_map_focus_{false};
    double current_grid_cell_length_m_{0.0};
};

}  // namespace map_panel
}  // namespace silverstar

#endif // MAP_GRAPHICSVIEW_H
