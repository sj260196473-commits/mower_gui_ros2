#include "mainwindow/map_panel/core/pnc_task_model.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

using silverstar::map_panel::EditableZone;
using silverstar::map_panel::EditableZoneKind;
using silverstar::map_panel::PncTaskType;

namespace
{
constexpr double kTolerance = 1e-9;

/// 简单断言工具，失败时输出消息并退出测试进程。
void expect(bool condition, const char* message)
{
    if (!condition) {
        std::cerr << message << std::endl;
        std::exit(1);
    }
}

/// 浮点近似断言工具，用于校验坐标和 yaw。
void expectNear(double actual, double expected, const char* message)
{
    expect(std::abs(actual - expected) < kTolerance, message);
}
}

/// 验证 P2P 与区域任务的结构化命令构造。
int main()
{
    const auto p2pTask = silverstar::map_panel::makeP2PTask(QPointF(1.25, -0.5), QPointF(1.25, 0.5));
    expect(p2pTask.type == NavigationPncTaskType::P2P, "p2p task should contain task type");
    expect(p2pTask.has_goal, "p2p task should contain a goal");
    expectNear(p2pTask.goal.x, 1.25, "p2p task should use press point as goal x");
    expectNear(p2pTask.goal.y, -0.5, "p2p task should use press point as goal y");
    expectNear(p2pTask.goal.yaw, M_PI / 2.0, "p2p task should derive yaw from press-to-release vector");

    EditableZone area;
    area.id = "area-7";
    area.name = "room";
    area.kind = EditableZoneKind::NoEntry;
    area.world_points = {QPointF(0.0, 0.0), QPointF(1.0, 0.0), QPointF(1.0, 1.0)};

    const auto coverageTask = silverstar::map_panel::makeZoneTask(PncTaskType::Coverage, area);
    expect(coverageTask.type == NavigationPncTaskType::Coverage, "coverage task should contain task type");
    expect(coverageTask.has_zone, "coverage task should contain a zone");
    expect(coverageTask.zone.id == "area-7", "coverage task should keep selected zone id");
    expect(coverageTask.zone.points.size() == 3, "coverage task should include polygon points");

    const auto alongWallTask = silverstar::map_panel::makeZoneTask(PncTaskType::AlongWall, area);
    expect(alongWallTask.type == NavigationPncTaskType::AlongWall,
           "along wall task should contain task type");

    return 0;
}
