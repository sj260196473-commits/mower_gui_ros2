#include "mainwindow/map_panel/core/pnc_task_model.h"

#include <cstdlib>
#include <iostream>

using silverstar::map_panel::EditableZone;
using silverstar::map_panel::EditableZoneKind;
using silverstar::map_panel::PncTaskType;

namespace
{
void expect(bool condition, const char* message)
{
    if (!condition) {
        std::cerr << message << std::endl;
        std::exit(1);
    }
}
}

int main()
{
    const QString p2pJson = silverstar::map_panel::serializeP2PTaskToJson(QPointF(1.25, -0.5));
    expect(p2pJson.contains("\"type\":\"P2P_GENERAL\""), "p2p task should contain task type");
    expect(p2pJson.contains("\"x\":1.25"), "p2p task should contain goal x");
    expect(p2pJson.contains("\"y\":-0.5"), "p2p task should contain goal y");

    EditableZone area;
    area.id = "area-7";
    area.name = "room";
    area.kind = EditableZoneKind::NoEntry;
    area.world_points = {QPointF(0.0, 0.0), QPointF(1.0, 0.0), QPointF(1.0, 1.0)};

    const QString coverageJson = silverstar::map_panel::serializeZoneTaskToJson(PncTaskType::Coverage, area);
    expect(coverageJson.contains("\"type\":\"COVERAGE_NAV\""), "coverage task should contain task type");
    expect(coverageJson.contains("\"zone_id\":\"area-7\""), "coverage task should keep selected zone id");
    expect(coverageJson.contains("\"points\""), "coverage task should include polygon points");

    const QString alongWallJson = silverstar::map_panel::serializeZoneTaskToJson(PncTaskType::AlongWall, area);
    expect(alongWallJson.contains("\"type\":\"ALONG_WALL\""), "along wall task should contain task type");

    return 0;
}
