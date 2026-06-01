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

EditableZoneLayerItem::EditableZoneLayerItem(const QString& id, const QString& name, const int& z, QGraphicsItem* parent)
    : MapLayerBase(id, name, "map", parent)
{
    setZValue(z);
}

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

QRectF EditableZoneLayerItem::boundingRect() const
{
    return scene_rect_;
}

void EditableZoneLayerItem::setSceneRect(const QRectF& rect)
{
    prepareGeometryChange();
    scene_rect_ = rect;
    update();
}

void EditableZoneLayerItem::setMapId(const QString& mapId)
{
    map_id_ = mapId;
}

QString EditableZoneLayerItem::mapId() const
{
    return map_id_;
}

void EditableZoneLayerItem::updateMap(const OccupancyMap& map)
{
    coordinate_transformer_.updateMap(map);
    if (map_id_.isEmpty()) {
        map_id_ = QStringLiteral("active-map");
    }
    update();
}

void EditableZoneLayerItem::beginDrawing(EditableZoneKind kind)
{
    drawing_kind_ = kind;
    drawing_points_.clear();
    drawing_ = true;
    selected_zone_id_.clear();
    update();
}

bool EditableZoneLayerItem::isDrawing() const
{
    return drawing_;
}

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

void EditableZoneLayerItem::cancelDrawing()
{
    drawing_points_.clear();
    drawing_ = false;
    update();
}

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

void EditableZoneLayerItem::clearZones()
{
    model_.clear();
    selected_zone_id_.clear();
    cancelDrawing();
    emit zonesChanged();
    emitCommitRequested();
}

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

const EditableZoneModel& EditableZoneLayerItem::model() const
{
    return model_;
}

QString EditableZoneLayerItem::planningZonesJson() const
{
    return serializeZonesToJson(model_.planningZones(), map_id_);
}

QString EditableZoneLayerItem::blockedAreasJson() const
{
    return serializeZonesToJson(model_.blockedAreas(), map_id_);
}

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
    }

    return QColor(220, 55, 55, alpha);
}

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

QString EditableZoneLayerItem::nextZoneId() const
{
    return QStringLiteral("gui-zone-%1").arg(next_id_);
}

void EditableZoneLayerItem::emitCommitRequested()
{
    emit zoneCommitRequested(planningZonesJson(), blockedAreasJson());
}

}  // namespace map_panel
}  // namespace silverstar
