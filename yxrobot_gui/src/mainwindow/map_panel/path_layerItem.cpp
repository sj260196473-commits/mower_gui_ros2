#include "path_layerItem.h"

PathLayerItem::PathLayerItem(const QString& id,const QString& name,const int& z,QGraphicsItem* parent)
    : MapLayerItemVirtual(id, name, "path", parent)
{
    setZValue(z);
    setAcceptHoverEvents(true);
}

void PathLayerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Implementation for painting the path layer item
    drawPath(painter, current_path_scene_);
}

QRectF PathLayerItem::boundingRect() const
{
    // Implementation for defining the bounding rectangle
    return bounding_rect_;
}

void PathLayerItem::computeBoundRect(const Path& path)
{
    if(path.waypoints.empty()) {
        bounding_rect_ = QRectF(); // No waypoints, set to default
        return;
    }
    
    //计算路径的边界矩形
    float xmax = path.waypoints[0].x;
    float xmin = path.waypoints[0].x;
    float ymax = path.waypoints[0].y;
    float ymin = path.waypoints[0].y;
    for(const auto& waypoint : path.waypoints) {
        if(waypoint.x > xmax) xmax = waypoint.x;
        if(waypoint.x < xmin) xmin = waypoint.x;
        if(waypoint.y > ymax) ymax = waypoint.y;
        if(waypoint.y < ymin) ymin = waypoint.y;
    }
    bounding_rect_ = QRectF(xmin, ymin, xmax - xmin, ymax - ymin);
}

void PathLayerItem::UpdatePath(const Path& path)
{
    std::cout<<"update path!"<<path.waypoints.size()<<std::endl;
    current_path_world_ = path;
    prepareGeometryChange();
    rebuildScenePath();
    update();
}

void PathLayerItem::updateMap(const OccupancyMap& map)
{
    map_ = map;
    prepareGeometryChange();
    rebuildScenePath();
    update();
}

void PathLayerItem::rebuildScenePath()
{
    current_path_scene_.header = current_path_world_.header;
    current_path_scene_.waypoints.clear();

    if (map_.isNULL()) {
        bounding_rect_ = QRectF();
        return;
    }

    current_path_scene_.waypoints.reserve(current_path_world_.waypoints.size());
    for (const auto& world_point : current_path_world_.waypoints) {
        Point scene_point;
        map_.worldPose2Scene(world_point.x, world_point.y, scene_point.x, scene_point.y);
        current_path_scene_.waypoints.push_back(scene_point);
    }

    computeBoundRect(current_path_scene_);
}

void PathLayerItem::drawPath(QPainter *painter, const Path& path)
{
    if(path.waypoints.empty()) return;

    QPen pen(Qt::blue);
    pen.setWidth(1);
    painter->setPen(pen);

    QPainterPath painter_path;
    painter_path.moveTo(path.waypoints[0].x, path.waypoints[0].y);
    for(size_t i = 1; i < path.waypoints.size(); ++i) {
        painter_path.lineTo(path.waypoints[i].x, path.waypoints[i].y);
    }
    painter->drawPath(painter_path);
}
