#ifndef EDITABLE_ZONE_MODEL_H
#define EDITABLE_ZONE_MODEL_H

#include "common/common.h"

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
    Furniture,
    CleanArea
};

struct EditableZone
{
    QString id;
    QString name;
    EditableZoneKind kind{EditableZoneKind::NoEntry};
    QVector<QPointF> world_points;
    bool enabled{true};
};

/// 将编辑区域类型映射为界面显示名称。
QString zoneKindDisplayName(EditableZoneKind kind);

/// 将 Qt 编辑模型中的区域类型转换为通用导航区域枚举。
NavigationZoneKind toNavigationZoneKind(EditableZoneKind kind);

/// 将单个编辑区域转换为非 Qt/ROS 的通用导航区域。
NavigationZone toNavigationZone(const EditableZone& zone);

/// 将编辑区域列表转换为带地图 id 的通用导航区域集合。
NavigationZoneCollection toNavigationZoneCollection(const QVector<EditableZone>& zones,
                                                    const QString& mapId);

class EditableZoneModel
{
public:
    /// 返回当前全部编辑区域。
    const QVector<EditableZone>& zones() const;

    /// 新增或按 id 替换一个编辑区域。
    void upsertZone(const EditableZone& zone);

    /// 删除指定 id 的区域，返回是否找到并删除。
    bool removeZone(const QString& id);

    /// 清空全部编辑区域。
    void clear();

    /// 返回全部区域的结构化通用导航集合。
    NavigationZoneCollection navigationZoneCollection(const QString& mapId) const;

private:
    QVector<EditableZone> zones_;
};

}  // namespace map_panel
}  // namespace silverstar

#endif // EDITABLE_ZONE_MODEL_H
