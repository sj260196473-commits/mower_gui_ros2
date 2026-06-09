#include "mainwindow/map_panel/core/editable_zone_model.h"

#include <cstdlib>
#include <iostream>

using silverstar::map_panel::EditableZone;
using silverstar::map_panel::EditableZoneKind;
using silverstar::map_panel::EditableZoneModel;

namespace
{
/// 简单断言工具，失败时输出消息并退出测试进程。
void expect(bool condition, const char* message)
{
    if (!condition) {
        std::cerr << message << std::endl;
        std::exit(1);
    }
}
}

/// 验证编辑区域模型的完整集合转换和删除行为。
int main()
{
    EditableZoneModel model;

    EditableZone noEntry;
    noEntry.id = "zone-1";
    noEntry.name = "no entry";
    noEntry.kind = EditableZoneKind::NoEntry;
    noEntry.world_points = {QPointF(0.0, 0.0), QPointF(1.0, 0.0), QPointF(1.0, 1.0)};
    model.upsertZone(noEntry);

    EditableZone furniture;
    furniture.id = "chair-1";
    furniture.name = "chair";
    furniture.kind = EditableZoneKind::Furniture;
    furniture.world_points = {QPointF(2.0, 2.0), QPointF(3.0, 2.0), QPointF(3.0, 3.0)};
    model.upsertZone(furniture);

    EditableZone cleanArea;
    cleanArea.id = "clean-1";
    cleanArea.name = "clean area";
    cleanArea.kind = EditableZoneKind::CleanArea;
    cleanArea.world_points = {QPointF(4.0, 4.0), QPointF(5.0, 4.0), QPointF(5.0, 5.0)};
    model.upsertZone(cleanArea);

    expect(model.zones().size() == 3, "model should keep three zones");

    const auto navigationZones = model.navigationZoneCollection("map-a");

    expect(navigationZones.map_id == "map-a", "navigation zones should keep map id");
    expect(navigationZones.zones.size() == 3, "navigation collection should contain every zone type");
    expect(navigationZones.zones[0].kind == NavigationZoneKind::NoEntry,
           "no entry zone should convert to navigation kind");
    expect(navigationZones.zones[0].points.size() == 3, "converted zone should keep points");
    expect(navigationZones.zones[1].kind == NavigationZoneKind::Furniture,
           "furniture should convert to navigation kind");
    expect(navigationZones.zones[1].id == "chair-1", "converted furniture should keep its id");
    expect(navigationZones.zones[2].kind == NavigationZoneKind::CleanArea,
           "clean area should convert to navigation kind");

    model.removeZone("chair-1");
    expect(model.zones().size() == 2, "removeZone should erase matching zone");

    return 0;
}
