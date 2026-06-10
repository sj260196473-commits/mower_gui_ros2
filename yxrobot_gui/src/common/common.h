#ifndef COMMON_H
#define COMMON_H
#include <cstdint>
#include <math.h>
#include <string>
#include <vector>
#include <Eigen/Dense>
#include "point.h"

typedef point<double> Point;

struct Header {
    std::string frame_id;
    int64_t stamp_ns{0};
};

struct RobotPose {
    double x{0};
    double y{0};
    double theta{0};
};

class OccupancyMap{
public:
    int m_rows{0};
    int m_cols{0};
    double m_resolution;
    Eigen::Vector3d m_origin;
    Eigen::MatrixXi m_map_data;

public:
    /// 构造空地图，行列数为 0 时表示无有效地图。
    OccupancyMap() {}

    /// 根据行列、分辨率和地图原点创建栅格地图数据容器。
    OccupancyMap(int rows,int cols,double resolution,Eigen::Vector3d origin)
        :m_rows(rows),m_cols(cols),m_resolution(resolution),m_origin(origin), m_map_data(m_rows, m_cols){}

    /// 默认拷贝构造，保留地图元数据和栅格矩阵。
    OccupancyMap(const OccupancyMap &other) = default;

    /// 默认拷贝赋值，复制地图元数据和栅格矩阵。
    OccupancyMap &operator=(const OccupancyMap &other) = default;

    /// 默认移动构造，用于高效传递地图数据。
    OccupancyMap(OccupancyMap &&other) noexcept = default;

    /// 默认移动赋值，用于高效替换地图数据。
    OccupancyMap &operator=(OccupancyMap &&other) noexcept = default;

    /// 默认析构，Eigen 矩阵自动释放资源。
    ~OccupancyMap() = default;

    /// 按行列索引返回可修改的栅格值。
    auto &operator()(int r, int c) { return m_map_data(r, c); }

    /// 按行列索引返回只读栅格值。
    const auto &operator()(int r, int c) const { return m_map_data(r, c); }

    /// 整体替换栅格矩阵数据。
    void SetMapData(const Eigen::MatrixXi &data) {
        m_map_data = data;
    }

    /// 判断地图是否为空，空地图不会参与绘制和坐标转换。
    bool isNULL() const
    {
        return (m_rows == 0 && m_cols == 0);
    }

    /// 返回地图矩阵行数。
    int Rows() const { return m_rows; }

    /// 返回地图矩阵列数。
    int Cols()  const { return m_cols; }

    //宽高地图坐标系下的长度
    /// 返回地图宽度，对应列数。
    int width()  const { return m_cols; }

    /// 返回地图高度，对应行数。
    int height()  const { return m_rows; }

    /// 返回地图分辨率，单位通常为 m/cell。
    double getRes() const {return m_resolution;}

    /// 返回地图原点
    Eigen::Vector3d getOrigin() const {return m_origin;}
};

struct LaserScan {
    /// 构造空激光点集合。
    LaserScan() = default;

    /// 使用 id 和点集合构造激光数据。
    LaserScan(int i, std::vector<Point> d) : id(i), data(d) {}

    /// 预留激光点容量，减少回调转换时的重复分配。
    void reserve(uint32_t length){data.reserve(length);}
    Header header;
    int id{0};
    std::vector<Point> data;

    /// 向激光点集合追加一个世界坐标点。
    void push_back(Point p) { data.push_back(p); }

    /// 清空当前激光点集合。
    void clear() { data.clear(); }
};

struct Path{
    Header header;
    std::vector<Point> waypoints;
};

enum class NavigationZoneKind
{
    NoEntry,
    VirtualWall,
    NoMop,
    NoSweep,
    Obstacle,
    Furniture,
    CleanArea
};

enum class NavigationPncTaskType
{
    P2P,
    Coverage,
    AlongWall
};

struct NavigationPoint2D
{
    double x{0.0};
    double y{0.0};
};

struct NavigationPose2D
{
    double x{0.0};
    double y{0.0};
    double yaw{0.0};
};

struct NavigationZone
{
    std::string id;
    std::string name;
    NavigationZoneKind kind{NavigationZoneKind::NoEntry};
    std::vector<NavigationPoint2D> points;
    bool enabled{true};
};

struct NavigationZoneCollection
{
    std::string map_id;
    std::vector<NavigationZone> zones;
};

struct NavigationPncTask
{
    std::string uuid;
    NavigationPncTaskType type{NavigationPncTaskType::P2P};
    NavigationPose2D goal;
    bool has_goal{false};
    NavigationZone zone;
    bool has_zone{false};
};

//角度转弧度
/// 将角度制转换为弧度制。
inline double deg2rad(double x) { return M_PI * x / 180.0; }
//弧度转角度
/// 将弧度制转换为角度制。
inline double rad2deg(double x) { return 180.0 * x / M_PI; }

#endif // COMMON_H
