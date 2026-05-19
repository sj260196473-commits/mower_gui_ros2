#ifndef COMMON_H
#define COMMON_H
#include <cstdint>
#include <math.h>
#include <Eigen/Dense>
#include "point.h"

typedef point<double> Point;

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

    /**
   * @description:
   * 输入世界坐标,返回原始(地图图片)图元坐标。
   * @param {double&} world_x
   * @param {double&} world_x
   * @param {double&} scene_x
   * @param {double&} scene_y
   * @return
   */
    void worldPose2Scene(const double &world_x,const double &world_y,
                         double &scene_x,double &scene_y)
    {
        scene_x = (world_x - m_origin[0]) / m_resolution;
        scene_y = (world_y - m_origin[1]) / m_resolution;
    }
    /**
   * @description:
   * 输入原始(地图图片)图元坐标,返回世界坐标
   * @param {double&} scene_x
   * @param {double&} scene_y
   * @param {double&} world_x
   * @param {double&} world_x
   * @return
   */
    void scenePose2World(const double &scene_x, const double &scene_y,
                         double &world_x, double &world_y)
    {
        world_x = scene_x * m_resolution + m_origin[0];
        world_y = scene_y * m_resolution + m_origin[1];
    }
    /**
   * @description: 获取带rgba颜色值的代价地图
   * @return {*}
   */
    Eigen::Matrix<Eigen::Vector4i, Eigen::Dynamic, Eigen::Dynamic> GetCostMapData()
    {
        Eigen::Matrix<Eigen::Vector4i, Eigen::Dynamic, Eigen::Dynamic> res =
            Eigen::Matrix<Eigen::Vector4i, Eigen::Dynamic, Eigen::Dynamic>(
                m_map_data.rows(), m_map_data.cols());
        for (int x = 0; x < m_map_data.rows(); x++) {
            for (int y = 0; y < m_map_data.cols(); y++) {
                // 计算像素值
                Eigen::Vector4i color_rgba;
                int data = m_map_data(x, y);
                if (data >= 100) {
                    color_rgba = Eigen::Vector4i(0xff, 0x00, 0xff, 50);

                } else if (data >= 90 && data < 100) {
                    color_rgba = Eigen::Vector4i(0x66, 0xff, 0xff, 50);

                } else if (data >= 70 && data <= 90) {
                    color_rgba = Eigen::Vector4i(0xff, 0x00, 0x33, 50);

                } else if (data >= 60 && data <= 70) {
                    color_rgba = Eigen::Vector4i(0xbe, 0x28, 0x1a, 50);

                } else if (data >= 50 && data < 60) {
                    color_rgba = Eigen::Vector4i(0xBE, 0x1F, 0x58, 50);

                } else if (data >= 40 && data < 50) {
                    color_rgba = Eigen::Vector4i(0xBE, 0x25, 0x76, 50);

                } else if (data >= 30 && data < 40) {
                    color_rgba = Eigen::Vector4i(0xBE, 0x2A, 0x99, 50);

                } else if (data >= 20 && data < 30) {
                    color_rgba = Eigen::Vector4i(0xBE, 0x35, 0xB3, 50);

                } else if (data >= 10 && data < 20) {
                    color_rgba = Eigen::Vector4i(0xB0, 0x3C, 0xbE, 50);

                } else {
                    // 其他 透明
                    color_rgba = Eigen::Vector4i(0, 0, 0, 0);
                }
                res(x, y) = color_rgba;
            }
        }
        return res;
    }
};

struct LaserScan {
    LaserScan() = default;
    LaserScan(int i, std::vector<Point> d) : id(i), data(d) {}
    void reserve(uint32_t length){data.reserve(length);}
    int id;
    std::vector<Point> data;
    void push_back(Point p) { data.push_back(p); }
    void clear() { data.clear(); }
};

struct Path{
    std::vector<Point> waypoints;
};

//角度转弧度
inline double deg2rad(double x) { return M_PI * x / 180.0; }
//弧度转角度
inline double rad2deg(double x) { return 180.0 * x / M_PI; }

#endif // COMMON_H
