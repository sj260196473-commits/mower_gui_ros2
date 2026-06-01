#ifndef EDITABLE_ZONE_MODEL_H
#define EDITABLE_ZONE_MODEL_H

#include <QPointF>
#include <QString>
#include <QVector>

namespace silverstar {
namespace map_panel {

enum class EditableZoneKind
{
    NoEntry,
    VirtualWall,
    NoMop,
    NoSweep,
    Obstacle,
    Furniture
};

struct EditableZone
{
    QString id;
    QString name;
    EditableZoneKind kind{EditableZoneKind::NoEntry};
    QVector<QPointF> world_points;
    bool enabled{true};
};

QString zoneKindToProtoName(EditableZoneKind kind);
QString zoneKindDisplayName(EditableZoneKind kind);
bool isBlockedAreaKind(EditableZoneKind kind);
QString serializeZonesToJson(const QVector<EditableZone>& zones, const QString& mapId);

class EditableZoneModel
{
public:
    const QVector<EditableZone>& zones() const;
    void upsertZone(const EditableZone& zone);
    bool removeZone(const QString& id);
    void clear();

    QVector<EditableZone> planningZones() const;
    QVector<EditableZone> blockedAreas() const;

private:
    QVector<EditableZone> zones_;
};

}  // namespace map_panel
}  // namespace silverstar

#endif // EDITABLE_ZONE_MODEL_H
