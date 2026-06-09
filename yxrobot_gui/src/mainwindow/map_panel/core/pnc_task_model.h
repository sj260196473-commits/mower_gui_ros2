#ifndef PNC_TASK_MODEL_H
#define PNC_TASK_MODEL_H

#include "common/common.h"
#include "mainwindow/map_panel/core/editable_zone_model.h"

#include <QPointF>
#include <QString>

namespace silverstar {
namespace map_panel {

enum class PncTaskType
{
    P2P,
    Coverage,
    AlongWall
};

/// 返回 PNC 任务类型的界面/协议名称。
QString pncTaskTypeName(PncTaskType type);

/// 根据按下点和释放点创建 P2P 目标位姿任务。
NavigationPncTask makeP2PTask(const QPointF& pressPoint, const QPointF& releasePoint);

/// 根据选中区域创建覆盖或沿墙任务。
NavigationPncTask makeZoneTask(PncTaskType type, const EditableZone& zone);

}  // namespace map_panel
}  // namespace silverstar

#endif // PNC_TASK_MODEL_H
