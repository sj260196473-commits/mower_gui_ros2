#include "mainwindow/map_panel/core/pnc_task_model.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace silverstar {
namespace map_panel {

namespace
{
QString makeUuid(const QString& prefix)
{
    return QStringLiteral("%1-%2").arg(prefix, QString::number(QDateTime::currentMSecsSinceEpoch()));
}

QJsonArray pointsToJson(const QVector<QPointF>& points)
{
    QJsonArray array;
    for (const QPointF& point : points) {
        QJsonObject object;
        object.insert(QStringLiteral("x"), point.x());
        object.insert(QStringLiteral("y"), point.y());
        array.append(object);
    }
    return array;
}
}

QString pncTaskTypeName(PncTaskType type)
{
    switch (type) {
    case PncTaskType::P2P:
        return QStringLiteral("P2P_GENERAL");
    case PncTaskType::Coverage:
        return QStringLiteral("COVERAGE_NAV");
    case PncTaskType::AlongWall:
        return QStringLiteral("ALONG_WALL");
    }

    return QStringLiteral("UNKNOWN");
}

QString serializeP2PTaskToJson(const QPointF& goal)
{
    QJsonObject root;
    root.insert(QStringLiteral("uuid"), makeUuid(QStringLiteral("p2p")));
    root.insert(QStringLiteral("type"), pncTaskTypeName(PncTaskType::P2P));

    QJsonObject goalObject;
    goalObject.insert(QStringLiteral("x"), goal.x());
    goalObject.insert(QStringLiteral("y"), goal.y());
    goalObject.insert(QStringLiteral("theta"), 0.0);
    root.insert(QStringLiteral("goal"), goalObject);

    return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact));
}

QString serializeZoneTaskToJson(PncTaskType type, const EditableZone& zone)
{
    QJsonObject root;
    root.insert(QStringLiteral("uuid"), makeUuid(type == PncTaskType::Coverage ? QStringLiteral("coverage") : QStringLiteral("alongwall")));
    root.insert(QStringLiteral("type"), pncTaskTypeName(type));
    root.insert(QStringLiteral("zone_id"), zone.id);
    root.insert(QStringLiteral("zone_name"), zone.name);
    root.insert(QStringLiteral("zone_kind"), zoneKindToProtoName(zone.kind));
    root.insert(QStringLiteral("points"), pointsToJson(zone.world_points));

    return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact));
}

}  // namespace map_panel
}  // namespace silverstar
