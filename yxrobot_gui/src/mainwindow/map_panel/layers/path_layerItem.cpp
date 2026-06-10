#include "path_layerItem.h"

namespace silverstar {
namespace map_panel {

/// 初始化路径图层元信息。
PathLayerItem::PathLayerItem(const QString& id,const QString& name,const int& z,QGraphicsItem* parent)
    : MapLayerBase(id, name, "path", parent)
{
    setZValue(z);
    setAcceptHoverEvents(true);
}

/// 绘制当前 scene 坐标路径。
void PathLayerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    // Implementation for painting the path layer item
    drawPath(painter, current_path_scene_);
}

/// 返回路径包围盒。
QRectF PathLayerItem::boundingRect() const
{
    // Implementation for defining the bounding rectangle
    return bounding_rect_;
}

/// 根据路径点计算最小外接矩形。
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

/// 保存新的世界坐标路径并重建 scene 缓存。
void PathLayerItem::UpdatePath(const Path& path)
{
    std::cout<<"update path!"<<path.waypoints.size()<<std::endl;
    current_path_world_ = path;
    prepareGeometryChange();
    rebuildScenePath();
    update();
}

/// 更新地图转换器，并用新地图参数重建路径。
void PathLayerItem::updateMap(const OccupancyMap& map)
{
    coordinate_transformer_.updateMap(map);
    prepareGeometryChange();
    rebuildScenePath();
    update();
}

/// 将世界坐标路径转换为 scene 坐标路径。
void PathLayerItem::rebuildScenePath()
{
    current_path_scene_.header = current_path_world_.header;
    current_path_scene_.waypoints.clear();

    if (!coordinate_transformer_.isValid()) {
        bounding_rect_ = QRectF();
        return;
    }

    current_path_scene_.waypoints.reserve(current_path_world_.waypoints.size());
    for (const auto& world_point : current_path_world_.waypoints) {
        current_path_scene_.waypoints.push_back(
            coordinate_transformer_.worldToScene(world_point));
    }

    computeBoundRect(current_path_scene_);
}

/// 按折线方式绘制路径点序列。
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

}  // namespace map_panel
}  // namespace silverstar
