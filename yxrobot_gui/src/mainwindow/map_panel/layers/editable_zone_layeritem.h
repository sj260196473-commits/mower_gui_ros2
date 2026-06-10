#ifndef EDITABLE_ZONE_LAYERITEM_H
#define EDITABLE_ZONE_LAYERITEM_H

#include "mainwindow/map_panel/core/editable_zone_model.h"
#include "mainwindow/map_panel/core/map_layeritem_virtual.h"
#include "common/common.h"
#include "common/map_coordinate_transformer.h"

#include <QRectF>

namespace silverstar {
namespace map_panel {

class EditableZoneLayerItem : public MapLayerBase
{
    Q_OBJECT
public:
    /// 构造编辑区域图层并设置 z 值。
    EditableZoneLayerItem(const QString& id, const QString& name, const int& z, QGraphicsItem* parent = nullptr);

    /// 绘制已提交区域、选中控制点和当前绘制中的折线。
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    /// 返回图层可绘制的场景范围。
    QRectF boundingRect() const override;

    /// 设置编辑层覆盖的 scene 范围。
    void setSceneRect(const QRectF& rect);

    /// 设置当前编辑区域所属地图 id。
    void setMapId(const QString& mapId);

    /// 返回当前地图 id。
    QString mapId() const;

    /// 更新坐标转换器的地图参数。
    void updateMap(const OccupancyMap& map);

    /// 进入指定类型区域的绘制状态。
    void beginDrawing(EditableZoneKind kind);

    /// 返回当前是否正在绘制区域。
    bool isDrawing() const;

    /// 向当前绘制区域追加一个世界坐标点。
    bool addWorldPoint(const QPointF& worldPoint);

    /// 尝试完成当前绘制，并提交到模型。
    bool finishDrawing();

    /// 取消当前绘制状态并清空临时点。
    void cancelDrawing();

    /// 根据世界坐标命中选择已有区域。
    bool selectAtWorldPoint(const QPointF& worldPoint);

    /// 删除当前选中的区域。
    bool removeSelectedZone();

    /// 清空所有编辑区域。
    void clearZones();

    /// 获取当前选中区域，未选中时返回 false。
    bool selectedZone(EditableZone* zone) const;

    /// 返回只读编辑区域模型。
    const EditableZoneModel& model() const;

    /// 返回全部编辑区域的结构化通用导航集合。
    NavigationZoneCollection navigationZoneCollection() const;

signals:
    /// 区域集合发生变化时发出。
    void zonesChanged();

    /// 请求外层把全部区域发送到通信通道。
    void zoneCommitRequested(const NavigationZoneCollection& zones);

private:
    /// 返回指定区域类型的绘制颜色。
    QColor colorForKind(EditableZoneKind kind, int alpha = 140) const;

    /// 将世界坐标点列表转换为 scene 坐标点列表。
    QVector<QPointF> toScenePoints(const QVector<QPointF>& worldPoints) const;

    /// 绘制单个区域及其选中状态。
    void drawZone(QPainter* painter, const EditableZone& zone, bool selected);

    /// 生成下一个 GUI 本地区域 id。
    QString nextZoneId() const;

    /// 发出区域提交请求信号。
    void emitCommitRequested();

private:
    QRectF scene_rect_;
    QString map_id_;
    MapCoordinateTransformer coordinate_transformer_;
    EditableZoneModel model_;
    EditableZoneKind drawing_kind_{EditableZoneKind::NoEntry};
    QVector<QPointF> drawing_points_;
    bool drawing_{false};
    QString selected_zone_id_;
    int next_id_{1};
};

}  // namespace map_panel
}  // namespace silverstar

#endif // EDITABLE_ZONE_LAYERITEM_H
