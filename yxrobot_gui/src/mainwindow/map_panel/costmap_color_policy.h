#ifndef COSTMAP_COLOR_POLICY_H
#define COSTMAP_COLOR_POLICY_H

#include <QColor>

class CostMapColorPolicy
{
public:
    QColor colorForCost(int cost) const
    {
        if (cost >= 100) {
            return QColor(0xff, 0x00, 0xff, 50);
        }
        if (cost >= 90) {
            return QColor(0x66, 0xff, 0xff, 50);
        }
        if (cost >= 70) {
            return QColor(0xff, 0x00, 0x33, 50);
        }
        if (cost >= 60) {
            return QColor(0xbe, 0x28, 0x1a, 50);
        }
        if (cost >= 50) {
            return QColor(0xbe, 0x1f, 0x58, 50);
        }
        if (cost >= 40) {
            return QColor(0xbe, 0x25, 0x76, 50);
        }
        if (cost >= 30) {
            return QColor(0xbe, 0x2a, 0x99, 50);
        }
        if (cost >= 20) {
            return QColor(0xbe, 0x35, 0xb3, 50);
        }
        if (cost >= 10) {
            return QColor(0xb0, 0x3c, 0xbe, 50);
        }
        return QColor(0, 0, 0, 0);
    }
};

#endif // COSTMAP_COLOR_POLICY_H
