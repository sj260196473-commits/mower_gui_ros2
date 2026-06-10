#include "editable_zone_layeritem.h"

#include <QLineF>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <algorithm>

namespace silverstar {
namespace map_panel {

namespace
{
constexpr qreal kPickDistance = 0.20;
constexpr qreal kHandleRadiusScene = 0.22;
}

/// 初始化编辑区域图层元信息。
EditableZoneLayerItem::EditableZoneLayerItem(const QString& id, const QString& name, const int& z, QGraphicsItem* parent)
    : MapLayerBase(id, name, "map", parent)
{
    setZValue(z);
}

/// 绘制所有已保存区域和当前正在绘制的临时点线。
void EditableZoneLayerItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing, true);
    for (const EditableZone& zone : model_.zones()) {
        drawZone(painter, zone, zone.id == selected_zone_id_);
    }

    if (!drawing_points_.isEmpty()) {
        const QVector<QPointF> scenePoints = toScenePoints(drawing_points_);
        QPen pen(colorForKind(drawing_kind_, 230));
        pen.setCosmetic(true);
        pen.setWidth(3);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);

        QPainterPath path;
        path.moveTo(scenePoints.front());
        for (int i = 1; i < scenePoints.size(); ++i) {
            path.lineTo(scenePoints[i]);
        }
        painter->drawPath(path);

        painter->setBrush(colorForKind(drawing_kind_, 230));
        for (const QPointF& point : scenePoints) {
            painter->drawEllipse(point, kHandleRadiusScene, kHandleRadiusScene);
        }
    }
}

/// 返回编辑图层覆盖的场景范围。
QRectF EditableZoneLayerItem::boundingRect() const
{
    return scene_rect_;
}

/// 设置图层覆盖范围，并通知 Qt 几何变化。
void EditableZoneLayerItem::setSceneRect(const QRectF& rect)
{
    prepareGeometryChange();
    scene_rect_ = rect;
    update();
}

/// 保存当前地图 id，用于提交区域时附带地图标识。
void EditableZoneLayerItem::setMapId(const QString& mapId)
{
    map_id_ = mapId;
}

/// 返回当前地图 id。
QString EditableZoneLayerItem::mapId() const
{
    return map_id_;
}

/// 更新世界坐标与场景坐标转换器。
void EditableZoneLayerItem::updateMap(const OccupancyMap& map)
{
    coordinate_transformer_.updateMap(map);
    if (map_id_.isEmpty()) {
        map_id_ = QStringLiteral("active-map");
    }
    update();
}

/// 开始绘制指定类型区域，并清空旧的临时绘制状态。
void EditableZoneLayerItem::beginDrawing(EditableZoneKind kind)
{
    drawing_kind_ = kind;
    drawing_points_.clear();
    drawing_ = true;
    selected_zone_id_.clear();
    update();
}

/// 返回当前是否处于绘制模式。
bool EditableZoneLayerItem::isDrawing() const
{
    return drawing_;
}

/// 追加一个世界坐标点；虚拟墙达到两个点后自动完成。
bool EditableZoneLayerItem::addWorldPoint(const QPointF& worldPoint)
{
    if (!drawing_) {
        return false;
    }

    drawing_points_.push_back(worldPoint);
    if (drawing_kind_ == EditableZoneKind::VirtualWall && drawing_points_.size() >= 2) {
        return finishDrawing();
    }

    update();
    return true;
}

/// 校验点数并把临时点提交为正式区域。
bool EditableZoneLayerItem::finishDrawing()
{
    if (!drawing_) {
        return false;
    }

    const int minPoints = drawing_kind_ == EditableZoneKind::VirtualWall ? 2 : 3;
    if (drawing_points_.size() < minPoints) {
        return false;
    }

    EditableZone zone;
    zone.id = nextZoneId();
    zone.name = zoneKindDisplayName(drawing_kind_);
    zone.kind = drawing_kind_;
    zone.world_points = drawing_points_;

    model_.upsertZone(zone);
    selected_zone_id_ = zone.id;
    drawing_points_.clear();
    drawing_ = false;
    ++next_id_;

    update();
    emit zonesChanged();
    emitCommitRequested();
    return true;
}

/// 放弃当前绘制过程。
void EditableZoneLayerItem::cancelDrawing()
{
    drawing_points_.clear();
    drawing_ = false;
    update();
}

/// 按世界坐标命中多边形或线段区域，并更新选中 id。
bool EditableZoneLayerItem::selectAtWorldPoint(const QPointF& worldPoint)
{
    QString foundId;
    for (const EditableZone& zone : model_.zones()) {
        if (zone.world_points.size() >= 3) {
            QPainterPath path;
            path.moveTo(zone.world_points.front());
            for (int i = 1; i < zone.world_points.size(); ++i) {
                path.lineTo(zone.world_points[i]);
            }
            path.closeSubpath();
            if (path.contains(worldPoint)) {
                foundId = zone.id;
            }
        } else if (zone.world_points.size() == 2) {
            const QLineF line(zone.world_points[0], zone.world_points[1]);
            const qreal distance =
                std::abs((line.p2().x() - line.p1().x()) * (line.p1().y() - worldPoint.y()) -
                         (line.p1().x() - worldPoint.x()) * (line.p2().y() - line.p1().y())) /
                std::max<qreal>(0.001, line.length());
            if (distance <= kPickDistance) {
                foundId = zone.id;
            }
        }
    }

    if (selected_zone_id_ == foundId) {
        return !foundId.isEmpty();
    }

    selected_zone_id_ = foundId;
    update();
    return !foundId.isEmpty();
}

/// 删除当前选中区域并提交最新集合。
bool EditableZoneLayerItem::removeSelectedZone()
{
    if (selected_zone_id_.isEmpty()) {
        return false;
    }

    const bool removed = model_.removeZone(selected_zone_id_);
    selected_zone_id_.clear();
    if (removed) {
        update();
        emit zonesChanged();
        emitCommitRequested();
    }
    return removed;
}

/// 清空所有区域和绘制状态，并提交空集合。
void EditableZoneLayerItem::clearZones()
{
    model_.clear();
    selected_zone_id_.clear();
    cancelDrawing();
    emit zonesChanged();
    emitCommitRequested();
}

/// 根据当前选中 id 返回区域副本。
bool EditableZoneLayerItem::selectedZone(EditableZone* zone) const
{
    if (selected_zone_id_.isEmpty()) {
        return false;
    }

    for (const EditableZone& current : model_.zones()) {
        if (current.id == selected_zone_id_) {
            if (zone) {
                *zone = current;
            }
            return true;
        }
    }

    return false;
}

/// 返回内部区域模型的只读引用。
const EditableZoneModel& EditableZoneLayerItem::model() const
{
    return model_;
}

/// 返回当前全部区域的结构化集合。
NavigationZoneCollection EditableZoneLayerItem::navigationZoneCollection() const
{
    return model_.navigationZoneCollection(map_id_);
}

/// 为不同区域类型提供半透明绘制颜色。
QColor EditableZoneLayerItem::colorForKind(EditableZoneKind kind, int alpha) const
{
    switch (kind) {
    case EditableZoneKind::NoEntry:
        return QColor(220, 55, 55, alpha);
    case EditableZoneKind::VirtualWall:
        return QColor(240, 80, 80, alpha);
    case EditableZoneKind::NoMop:
        return QColor(45, 145, 255, alpha);
    case EditableZoneKind::NoSweep:
        return QColor(246, 172, 62, alpha);
    case EditableZoneKind::Obstacle:
        return QColor(120, 90, 70, alpha);
    case EditableZoneKind::Furniture:
        return QColor(95, 155, 100, alpha);
    case EditableZoneKind::CleanArea:
        return QColor(52, 190, 120, alpha);
    }

    return QColor(220, 55, 55, alpha);
}

/// 将世界坐标点批量转换为 scene 坐标点。
QVector<QPointF> EditableZoneLayerItem::toScenePoints(const QVector<QPointF>& worldPoints) const
{
    QVector<QPointF> scenePoints;
    scenePoints.reserve(worldPoints.size());
    for (const QPointF& point : worldPoints) {
        if (coordinate_transformer_.isValid()) {
            const Point scenePoint = coordinate_transformer_.worldToScene(Point(point.x(), point.y()));
            scenePoints.push_back(QPointF(scenePoint.x, scenePoint.y));
        } else {
            scenePoints.push_back(point);
        }
    }
    return scenePoints;
}

/// 绘制一个区域，选中时额外绘制白色控制点。
void EditableZoneLayerItem::drawZone(QPainter* painter, const EditableZone& zone, bool selected)
{
    const QVector<QPointF> scenePoints = toScenePoints(zone.world_points);
    const QColor color = colorForKind(zone.kind, selected ? 190 : 125);
    QPen pen(colorForKind(zone.kind, 235));
    pen.setCosmetic(true);
    pen.setWidth(selected ? 4 : 3);
    painter->setPen(pen);
    painter->setBrush(QBrush(color));

    if (scenePoints.size() >= 3) {
        painter->drawPolygon(QPolygonF(scenePoints));
    } else if (scenePoints.size() == 2) {
        painter->drawLine(scenePoints[0], scenePoints[1]);
    }

    if (selected) {
        painter->setBrush(Qt::white);
        for (const QPointF& point : scenePoints) {
            painter->drawEllipse(point, kHandleRadiusScene, kHandleRadiusScene);
        }
    }
}

/// 根据递增计数生成 GUI 本地 id。
QString EditableZoneLayerItem::nextZoneId() const
{
    return QStringLiteral("gui-zone-%1").arg(next_id_);
}

/// 发送全部结构化区域集合。
void EditableZoneLayerItem::emitCommitRequested()
{
    emit zoneCommitRequested(navigationZoneCollection());
}

}  // namespace map_panel
}  // namespace silverstar
