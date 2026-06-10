#include "mainwindow/map_panel/layers/robotpose_layerItem.h"

#include <Eigen/Dense>
#include <QApplication>
#include <QImage>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <cstdlib>
#include <cmath>
#include <iostream>

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

/// 浮点近似断言工具，用于校验绘制包围盒尺寸。
void expectNear(double actual, double expected, double tolerance, const char* message)
{
    if (std::abs(actual - expected) > tolerance) {
        std::cerr << message << ": actual=" << actual << ", expected=" << expected << std::endl;
        std::exit(1);
    }
}

/// 统计指定区域内明显属于红色前向轴的像素数量。
int countRedPixels(const QImage& image, const QRect& rect)
{
    int count = 0;
    for (int y = rect.top(); y <= rect.bottom(); ++y) {
        for (int x = rect.left(); x <= rect.right(); ++x) {
            const QColor color = image.pixelColor(x, y);
            if (color.alpha() > 80 && color.red() > color.green() * 1.5 && color.red() > color.blue() * 1.5) {
                ++count;
            }
        }
    }
    return count;
}

/// 统计指定区域内明显属于绿色侧向轴的像素数量。
int countGreenPixels(const QImage& image, const QRect& rect)
{
    int count = 0;
    for (int y = rect.top(); y <= rect.bottom(); ++y) {
        for (int x = rect.left(); x <= rect.right(); ++x) {
            const QColor color = image.pixelColor(x, y);
            if (color.alpha() > 80 && color.green() > color.red() * 1.5 && color.green() > color.blue() * 1.5) {
                ++count;
            }
        }
    }
    return count;
}

/// 将机器人图层按当前尺寸绘制到透明图像中。
QImage renderRobot(silverstar::map_panel::RobotPoseItem& item, int side)
{
    QImage image(side, side, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.translate(side / 2, side / 2);
    QStyleOptionGraphicsItem option;
    item.paint(&painter, &option, nullptr);
    painter.end();
    return image;
}
}

/// 验证机器人图层尺寸换算和基本绘制颜色分布。
int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    silverstar::map_panel::RobotPoseItem item("robot", "Robot", 1);
    const OccupancyMap map(100, 100, 0.05, Eigen::Vector3d(0.0, 0.0, 0.0));

    item.updateMap(map);
    const QRectF defaultRect = item.boundingRect();
    expect(defaultRect.width() > 10.0, "default footprint should include arrow padding");
    expect(defaultRect.height() >= 10.0, "default footprint should scale from meters to map pixels");

    item.setRobotRadius(1.0);
    const QRectF resizedRect = item.boundingRect();
    expect(resizedRect.width() > defaultRect.width(), "wider robot size should expand bounding rect");
    expectNear(resizedRect.width(), resizedRect.height(), 0.001,
               "circular footprint and perpendicular axes should keep the bounding rect square");

    QImage image(120, 120, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.translate(60, 60);
    QStyleOptionGraphicsItem option;
    item.paint(&painter, &option, nullptr);
    painter.end();

    const QColor center = image.pixelColor(60, 60);
    const QColor redAxis = image.pixelColor(74, 60);
    const QColor greenAxis = image.pixelColor(60, 74);
    const QColor diagonalGap = image.pixelColor(80, 80);
    expect(center.blue() > center.red() && center.blue() > center.green(),
           "robot origin should be a blue joint dot");
    expect(redAxis.red() > redAxis.green() && redAxis.red() > redAxis.blue(),
           "red axis should be a compact bar along robot forward +X");
    expect(greenAxis.green() > greenAxis.red() && greenAxis.green() > greenAxis.blue(),
           "green axis should be a compact bar along robot +Y and stay perpendicular to +X");
    expect(countRedPixels(image, QRect(96, 58, 4, 4)) > 0,
           "red forward arrow should extend to about two robot radii");
    expect(countGreenPixels(image, QRect(58, 96, 4, 4)) > 0,
           "green +Y arrow should extend to about two robot radii");
    expect(diagonalGap.alpha() < 80,
           "diagonal space should stay mostly clear instead of being filled by oversized arrow triangles");

    item.setRobotRadius(0.1775);
    const QImage defaultRobotImage = renderRobot(item, 80);
    const int defaultRobotRedPixels = countRedPixels(defaultRobotImage, QRect(40, 37, 5, 6));
    const int defaultRobotGreenPixels = countGreenPixels(defaultRobotImage, QRect(37, 40, 6, 5));
    expect(defaultRobotRedPixels > 0,
           "default robot should keep the red forward axis visible inside the footprint");
    expect(defaultRobotGreenPixels > 0,
           "default robot should keep the green side axis visible inside the footprint");

    return 0;
}
