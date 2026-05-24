#include "mainwindow/map_panel/laser_layeritem.h"
#include <iostream>

LaserItem::LaserItem(const QString& id,const QString& name,const int& z,QGraphicsItem* parent)
    :MapLayerBase(id,name,"scan",parent)
{
    setZValue(z);
    setAcceptHoverEvents(true);
}

void LaserItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    for (auto [id, data] : laser_data_scene_) {
        drawLaser(painter, id, data);
    }
}

QRectF LaserItem::boundingRect() const
{
    return bounding_rect_;
}

void LaserItem::computeBoundRect(const std::map<int, std::vector<Point>> &laser_scan) {
    if (laser_scan.empty()) {
        bounding_rect_ = QRectF();
        return;
    }

    bool first_point = true;
    float xmax = 0, xmin = 0, ymax = 0, ymin = 0;

    for (const auto& [id, points] : laser_scan) {
        for (const auto& p : points) {
            if (!std::isfinite(p.x) || !std::isfinite(p.y)) {
                continue;
            }

            if (first_point) {
                xmax = xmin = p.x;
                ymax = ymin = p.y;
                first_point = false;
            } else {
                xmax = std::max(xmax, static_cast<float>(p.x));
                xmin = std::min(xmin, static_cast<float>(p.x));
                ymax = std::max(ymax, static_cast<float>(p.y));
                ymin = std::min(ymin, static_cast<float>(p.y));
            }
        }
    }

    if (first_point) { // 防止所有的 points 数组都是空的
        bounding_rect_ = QRectF();
    } else {
        // QRectF(x, y, width, height)
        bounding_rect_ = QRectF(xmin, ymin, xmax - xmin, ymax - ymin);
    }
}


void LaserItem::UpdateLaserData(const LaserScan& scan) {
    // std::cout<<"update laser!"<<std::endl;
    laser_data_world_[scan.id] = scan.data;
    laser_data_scene_[scan.id] = convertWorldPointsToScene(scan.data);
    prepareGeometryChange();
    computeBoundRect(laser_data_scene_);
    update();
}

void LaserItem::updateMap(const OccupancyMap& map)
{
    coordinate_transformer_.updateMap(map);
    prepareGeometryChange();
    rebuildSceneLaserData();
    update();
}

std::vector<Point> LaserItem::convertWorldPointsToScene(const std::vector<Point>& world_points) const
{
    std::vector<Point> scene_points;
    // 只有当收到全局地图时，坐标转换器才有效，否则直接返回空
    if (!coordinate_transformer_.isValid()) {
        return scene_points;
    }

    scene_points.reserve(world_points.size());
    for (const auto& world_point : world_points) {
        scene_points.push_back(coordinate_transformer_.worldToScene(world_point));
    }
    return scene_points;
}

void LaserItem::rebuildSceneLaserData()
{
    laser_data_scene_.clear();
    for (const auto& [id, world_points] : laser_data_world_) {
        laser_data_scene_[id] = convertWorldPointsToScene(world_points);
    }
    computeBoundRect(laser_data_scene_);
}

void LaserItem::drawLaser(QPainter *painter, int id, const std::vector<Point>& data) {
    if (data.empty()) return;

    QColor color;
    auto it = location_to_color_.find(id);
    if (it == location_to_color_.end()) {
        color = Id2Color(id);
        location_to_color_[id] = color;
    } else {
        color = it->second;
    }
    QPen pen(color);
    pen.setWidth(3);        // 宽度设为 3 个像素，确保肉眼清晰可见
    pen.setCosmetic(true);  // 无论怎么放大缩小，永远是屏幕上的 3x3 像素点
    painter->setPen(pen);

    QVector<QPointF> qpoints;
    qpoints.reserve(data.size()); // 提前分配内存
    for (const auto& one_point : data) {
        qpoints.append(QPointF(one_point.x, one_point.y));
    }

    painter->drawPoints(qpoints.data(), qpoints.size());
}

QColor LaserItem::Id2Color(int id) {
    // 提取 RGB 颜色的小型 lambda 函数
    auto hexToColor = [](uint32_t hex) {
        return QColor((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF);
    };

    switch (id) {
        case 0: return hexToColor(0xFF6347);
        case 1: return hexToColor(0xFF6600);
        case 2: return hexToColor(0x228B22);
        case 3: return hexToColor(0x800000);
        case 4: return hexToColor(0x8A2BE2);
        case 5: return hexToColor(0xF4A460);
        case 6: return hexToColor(0xD2B48C);
        case 7: return hexToColor(0xADFF2F);
        case 8: return hexToColor(0xFF00FF);
        case 9: return hexToColor(0x00FF00);
        case -1: return hexToColor(0x40E0D0);
        case -2: return hexToColor(0x2F4F4F);
        case -3: return hexToColor(0x00BFFF);
        case -4: return hexToColor(0x708090);
        case -5: return hexToColor(0x00008B);
        case -6: return hexToColor(0x006633);
        case -7: return hexToColor(0x003300);
        case -8: return hexToColor(0xDA70D6);
        case -9: return hexToColor(0x9900cc);
        case -20: return hexToColor(0x551A8B);
        case 10: return hexToColor(0x00FF33);
    default:
        return QColor(Qt::white); // 必须有一个默认颜色兜底
    }
}
