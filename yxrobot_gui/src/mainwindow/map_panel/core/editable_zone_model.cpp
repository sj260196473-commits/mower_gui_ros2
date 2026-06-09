#include "mainwindow/map_panel/core/editable_zone_model.h"

namespace silverstar {
namespace map_panel {

/// 将编辑区域类型转换为用户可读的英文显示名。
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
    case EditableZoneKind::CleanArea:
        return QStringLiteral("Clean Area");
    }

    return QStringLiteral("Zone");
}

/// 将 Qt 编辑层枚举转换为 common 通用导航枚举。
NavigationZoneKind toNavigationZoneKind(EditableZoneKind kind)
{
    switch (kind) {
    case EditableZoneKind::NoEntry:
        return NavigationZoneKind::NoEntry;
    case EditableZoneKind::VirtualWall:
        return NavigationZoneKind::VirtualWall;
    case EditableZoneKind::NoMop:
        return NavigationZoneKind::NoMop;
    case EditableZoneKind::NoSweep:
        return NavigationZoneKind::NoSweep;
    case EditableZoneKind::Obstacle:
        return NavigationZoneKind::Obstacle;
    case EditableZoneKind::Furniture:
        return NavigationZoneKind::Furniture;
    case EditableZoneKind::CleanArea:
        return NavigationZoneKind::CleanArea;
    }

    return NavigationZoneKind::NoEntry;
}

/// 将一个 EditableZone 转为不依赖 Qt 的通用导航区域。
NavigationZone toNavigationZone(const EditableZone& zone)
{
    NavigationZone result;
    result.id = zone.id.toStdString();
    result.name = zone.name.toStdString();
    result.kind = toNavigationZoneKind(zone.kind);
    result.enabled = zone.enabled;
    result.points.reserve(zone.world_points.size());
    for (const QPointF& point : zone.world_points) {
        result.points.push_back({point.x(), point.y()});
    }
    return result;
}

/// 将区域列表整体转为通用导航集合，并附带地图 id。
NavigationZoneCollection toNavigationZoneCollection(const QVector<EditableZone>& zones,
                                                    const QString& mapId)
{
    NavigationZoneCollection result;
    result.map_id = mapId.toStdString();
    result.zones.reserve(zones.size());
    for (const EditableZone& zone : zones) {
        result.zones.push_back(toNavigationZone(zone));
    }
    return result;
}

/// 返回内部区域列表的只读引用，供绘制和查询使用。
const QVector<EditableZone>& EditableZoneModel::zones() const
{
    return zones_;
}

/// 按 id 更新已有区域，未找到时追加到列表末尾。
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

/// 删除指定 id 的区域，并返回删除是否成功。
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

/// 清空全部区域数据。
void EditableZoneModel::clear()
{
    zones_.clear();
}

/// 获取全部区域的结构化通用导航集合。
NavigationZoneCollection EditableZoneModel::navigationZoneCollection(const QString& mapId) const
{
    return toNavigationZoneCollection(zones_, mapId);
}

}  // namespace map_panel
}  // namespace silverstar
