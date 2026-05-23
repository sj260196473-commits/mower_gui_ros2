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
    OccupancyMap() {}
    OccupancyMap(int rows,int cols,double resolution,Eigen::Vector3d origin)
        :m_rows(rows),m_cols(cols),m_resolution(resolution),m_origin(origin), m_map_data(m_rows, m_cols){}
    OccupancyMap(const OccupancyMap &other) = default;
    OccupancyMap &operator=(const OccupancyMap &other) = default;
    OccupancyMap(OccupancyMap &&other) noexcept = default;
    OccupancyMap &operator=(OccupancyMap &&other) noexcept = default;
    ~OccupancyMap() = default;

    auto &operator()(int r, int c) { return m_map_data(r, c); }
    const auto &operator()(int r, int c) const { return m_map_data(r, c); }
    void SetMapData(const Eigen::MatrixXi &data) {
        m_map_data = data;
    }

    bool isNULL() const
    {
        return (m_rows == 0 && m_cols == 0);
    }

    int Rows() const { return m_rows; }
    int Cols()  const { return m_cols; }
    //宽高地图坐标系下的长度
    int width()  const { return m_cols; }
    int height()  const { return m_rows; }
    double getRes() const {return m_resolution;}
};

struct LaserScan {
    LaserScan() = default;
    LaserScan(int i, std::vector<Point> d) : id(i), data(d) {}
    void reserve(uint32_t length){data.reserve(length);}
    Header header;
    int id{0};
    std::vector<Point> data;
    void push_back(Point p) { data.push_back(p); }
    void clear() { data.clear(); }
};

struct Path{
    Header header;
    std::vector<Point> waypoints;
};

//角度转弧度
inline double deg2rad(double x) { return M_PI * x / 180.0; }
//弧度转角度
inline double rad2deg(double x) { return 180.0 * x / M_PI; }

#endif // COMMON_H
