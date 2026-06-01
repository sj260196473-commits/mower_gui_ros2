#include "mainwindow/map_panel/core/editable_zone_model.h"

#include <cstdlib>
#include <iostream>

using silverstar::map_panel::EditableZone;
using silverstar::map_panel::EditableZoneKind;
using silverstar::map_panel::EditableZoneModel;

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

    expect(model.zones().size() == 2, "model should keep two zones");
    expect(model.planningZones().size() == 1, "hard planning zones should exclude furniture");
    expect(model.blockedAreas().size() == 1, "blocked areas should include furniture");

    const QString zoneJson = silverstar::map_panel::serializeZonesToJson(model.planningZones(), "map-a");
    const QString blockedJson = silverstar::map_panel::serializeZonesToJson(model.blockedAreas(), "map-a");

    expect(zoneJson.contains("\"type\":\"NO_ENTRY_ZONE\""), "no entry zone should serialize as NO_ENTRY_ZONE");
    expect(blockedJson.contains("\"type\":\"BLOCKED_ZONE\""), "furniture should serialize as BLOCKED_ZONE");
    expect(blockedJson.contains("\"id\":\"chair-1\""), "serialized furniture should keep its id");

    model.removeZone("chair-1");
    expect(model.zones().size() == 1, "removeZone should erase matching zone");

    return 0;
}
