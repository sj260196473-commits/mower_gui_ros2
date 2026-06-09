#include "mainwindow/map_panel/core/pnc_task_model.h"

#include <QDateTime>
#include <cmath>

namespace silverstar {
namespace map_panel {

namespace
{
/// 根据前缀和当前毫秒时间生成简单任务 uuid。
QString makeUuid(const QString& prefix)
{
    return QStringLiteral("%1-%2").arg(prefix, QString::number(QDateTime::currentMSecsSinceEpoch()));
}

/// 将 UI 层 PNC 枚举转换为通用导航 PNC 枚举。
NavigationPncTaskType toNavigationPncTaskType(PncTaskType type)
{
    switch (type) {
    case PncTaskType::P2P:
        return NavigationPncTaskType::P2P;
    case PncTaskType::Coverage:
        return NavigationPncTaskType::Coverage;
    case PncTaskType::AlongWall:
        return NavigationPncTaskType::AlongWall;
    }

    return NavigationPncTaskType::P2P;
}
}

/// 返回 PNC 任务类型对应的业务字符串。
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

/// 根据 RViz 风格拖拽起点/终点生成点到点目标位姿任务。
NavigationPncTask makeP2PTask(const QPointF& pressPoint, const QPointF& releasePoint)
{
    NavigationPncTask task;
    task.uuid = makeUuid(QStringLiteral("p2p")).toStdString();
    task.type = NavigationPncTaskType::P2P;
    task.goal = {
        pressPoint.x(),
        pressPoint.y(),
        std::atan2(releasePoint.y() - pressPoint.y(), releasePoint.x() - pressPoint.x())
    };
    task.has_goal = true;
    return task;
}

/// 根据选中区域生成覆盖或沿墙任务。
NavigationPncTask makeZoneTask(PncTaskType type, const EditableZone& zone)
{
    NavigationPncTask task;
    task.uuid = makeUuid(type == PncTaskType::Coverage ? QStringLiteral("coverage") : QStringLiteral("alongwall")).toStdString();
    task.type = toNavigationPncTaskType(type);
    task.zone = toNavigationZone(zone);
    task.has_zone = true;
    return task;
}

}  // namespace map_panel
}  // namespace silverstar
