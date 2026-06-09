#ifndef LASER_LAYERITEM_H
#define LASER_LAYERITEM_H
#include "mainwindow/map_panel/core/map_layeritem_virtual.h"
#include "common/common.h"
#include "common/map_coordinate_transformer.h"
#include <QPainter>
#include <QColor>
#include <QGraphicsItem>
#include <QGraphicsSceneWheelEvent>
#include <map>

namespace silverstar {
namespace map_panel {

class LaserItem : public MapLayerBase
{
public:
    /// 构造激光图层并设置 z 值。
    LaserItem(const QString& id,const QString& name,const int& z,QGraphicsItem* parent = nullptr);

    /// 绘制当前缓存的 scene 坐标激光点。
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;

    /// 返回激光点云包围盒。
    QRectF boundingRect() const;

    /// 更新地图参数并重建 scene 坐标激光缓存。
    void updateMap(const OccupancyMap& map);

    /// 接收新的世界坐标激光数据。
    void UpdateLaserData(const LaserScan& scan);

private:
    /// 根据激光 id 分配稳定颜色。
    QColor Id2Color(int id);

    /// 绘制某一路激光点集合。
    void drawLaser(QPainter *painter, int id, const std::vector<Point>& data);

    /// 根据激光点集合计算图层包围盒。
    void computeBoundRect(const std::map<int, std::vector<Point>> &laser_scan);

    /// 将世界坐标点集合转换为 scene 坐标点集合。
    std::vector<Point> convertWorldPointsToScene(const std::vector<Point>& world_points) const;

    /// 使用最新地图参数重建全部激光 scene 缓存。
    void rebuildSceneLaserData();

private:
    MapCoordinateTransformer coordinate_transformer_;
    std::map<int, QColor> location_to_color_;
    std::map<int, std::vector<Point>> laser_data_world_;
    std::map<int, std::vector<Point>> laser_data_scene_;
    QRectF bounding_rect_;
};

}  // namespace map_panel
}  // namespace silverstar

#endif // LASER_LAYERITEM_H
