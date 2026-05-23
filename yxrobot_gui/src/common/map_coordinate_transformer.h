#ifndef MAP_COORDINATE_TRANSFORMER_H
#define MAP_COORDINATE_TRANSFORMER_H

#include "common/common.h"

class MapCoordinateTransformer
{
public:
    MapCoordinateTransformer() = default;

    explicit MapCoordinateTransformer(const OccupancyMap& map)
    {
        updateMap(map);
    }

    void updateMap(const OccupancyMap& map)
    {
        valid_ = !map.isNULL() && map.getRes() > 0.0;
        if (!valid_) {
            return;
        }

        resolution_ = map.getRes();
        origin_x_ = map.m_origin[0];
        origin_y_ = map.m_origin[1];
    }

    bool isValid() const
    {
        return valid_;
    }

    Point worldToScene(const Point& world_point) const
    {
        Point scene_point;
        worldToScene(world_point.x, world_point.y, scene_point.x, scene_point.y);
        return scene_point;
    }

    void worldToScene(
        const double& world_x,
        const double& world_y,
        double& scene_x,
        double& scene_y) const
    {
        scene_x = (world_x - origin_x_) / resolution_;
        scene_y = (world_y - origin_y_) / resolution_;
    }

    Point sceneToWorld(const Point& scene_point) const
    {
        Point world_point;
        sceneToWorld(scene_point.x, scene_point.y, world_point.x, world_point.y);
        return world_point;
    }

    void sceneToWorld(
        const double& scene_x,
        const double& scene_y,
        double& world_x,
        double& world_y) const
    {
        world_x = scene_x * resolution_ + origin_x_;
        world_y = scene_y * resolution_ + origin_y_;
    }

private:
    bool valid_{false};
    double resolution_{1.0};
    double origin_x_{0.0};
    double origin_y_{0.0};
};

#endif // MAP_COORDINATE_TRANSFORMER_H
