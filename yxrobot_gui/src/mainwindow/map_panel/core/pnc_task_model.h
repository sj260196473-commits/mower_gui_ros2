#ifndef PNC_TASK_MODEL_H
#define PNC_TASK_MODEL_H

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

QString pncTaskTypeName(PncTaskType type);
QString serializeP2PTaskToJson(const QPointF& goal);
QString serializeZoneTaskToJson(PncTaskType type, const EditableZone& zone);

}  // namespace map_panel
}  // namespace silverstar

#endif // PNC_TASK_MODEL_H
