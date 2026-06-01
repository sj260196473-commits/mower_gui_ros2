#include "mainwindow/map_panel/core/editable_zone_model.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace silverstar {
namespace map_panel {

QString zoneKindToProtoName(EditableZoneKind kind)
{
    switch (kind) {
    case EditableZoneKind::NoEntry:
        return QStringLiteral("NO_ENTRY_ZONE");
    case EditableZoneKind::VirtualWall:
        return QStringLiteral("VIRTUAL_WALL");
    case EditableZoneKind::NoMop:
        return QStringLiteral("NO_MOP_ZONE");
    case EditableZoneKind::NoSweep:
        return QStringLiteral("NO_SWEEP_ZONE");
    case EditableZoneKind::Obstacle:
    case EditableZoneKind::Furniture:
        return QStringLiteral("BLOCKED_ZONE");
    }

    return QStringLiteral("NO_ENTRY_ZONE");
}

QString zoneKindDisplayName(EditableZoneKind kind)
{
    switch (kind) {
    case EditableZoneKind::NoEntry:
        return QStringLiteral("No Entry");
    case EditableZoneKind::VirtualWall:
        return QStringLiteral("Virtual Wall");
    case EditableZoneKind::NoMop:
        return QStringLiteral("No Mop");
    case EditableZoneKind::NoSweep:
        return QStringLiteral("No Sweep");
    case EditableZoneKind::Obstacle:
        return QStringLiteral("Obstacle");
    case EditableZoneKind::Furniture:
        return QStringLiteral("Furniture");
    }

    return QStringLiteral("Zone");
}

bool isBlockedAreaKind(EditableZoneKind kind)
{
    return kind == EditableZoneKind::Obstacle || kind == EditableZoneKind::Furniture;
}

QString serializeZonesToJson(const QVector<EditableZone>& zones, const QString& mapId)
{
    QJsonObject root;
    root.insert(QStringLiteral("map_id"), mapId);

    QJsonArray zoneArray;
    for (const EditableZone& zone : zones) {
        if (!zone.enabled) {
            continue;
        }

        QJsonObject zoneObject;
        zoneObject.insert(QStringLiteral("id"), zone.id);
        zoneObject.insert(QStringLiteral("name"), zone.name);
        zoneObject.insert(QStringLiteral("type"), zoneKindToProtoName(zone.kind));
        zoneObject.insert(QStringLiteral("action"), QStringLiteral("ADD"));

        QJsonArray pointsArray;
        for (const QPointF& point : zone.world_points) {
            QJsonObject pointObject;
            pointObject.insert(QStringLiteral("x"), point.x());
            pointObject.insert(QStringLiteral("y"), point.y());
            pointsArray.append(pointObject);
        }
        zoneObject.insert(QStringLiteral("points"), pointsArray);
        zoneArray.append(zoneObject);
    }

    root.insert(QStringLiteral("zones"), zoneArray);
    return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact));
}

const QVector<EditableZone>& EditableZoneModel::zones() const
{
    return zones_;
}

void EditableZoneModel::upsertZone(const EditableZone& zone)
{
    for (EditableZone& existing : zones_) {
        if (existing.id == zone.id) {
            existing = zone;
            return;
        }
    }
    zones_.push_back(zone);
}

bool EditableZoneModel::removeZone(const QString& id)
{
    for (int i = 0; i < zones_.size(); ++i) {
        if (zones_[i].id == id) {
            zones_.removeAt(i);
            return true;
        }
    }
    return false;
}

void EditableZoneModel::clear()
{
    zones_.clear();
}

QVector<EditableZone> EditableZoneModel::planningZones() const
{
    QVector<EditableZone> result;
    for (const EditableZone& zone : zones_) {
        if (!isBlockedAreaKind(zone.kind)) {
            result.push_back(zone);
        }
    }
    return result;
}

QVector<EditableZone> EditableZoneModel::blockedAreas() const
{
    QVector<EditableZone> result;
    for (const EditableZone& zone : zones_) {
        if (isBlockedAreaKind(zone.kind)) {
            result.push_back(zone);
        }
    }
    return result;
}

}  // namespace map_panel
}  // namespace silverstar
