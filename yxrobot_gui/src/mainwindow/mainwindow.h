#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QPointF>
#include <QPushButton>
#include <memory>
#include "channel/channel_manager.h"
#include "common/common.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// 构造主窗口，初始化 UI、状态栏和通信通道。
    MainWindow(QWidget *parent = nullptr);

    /// 释放 UI 资源。
    ~MainWindow();

private slots:
    /// 切换右侧 dock 面板的显示状态。
    void on_hide_right_btn_clicked();

    /// 切换左侧 dock 面板的显示状态。
    void on_hide_left_btn_clicked();

private:
    /// 创建状态栏控件，并连接地图视图状态信号。
    void setupStatusBar();

    /// 将鼠标 scene/world 坐标显示到状态栏。
    void updateMousePositionStatus(const QPointF& scene_pos, const QPointF& world_pos, bool has_world);

    /// 将当前网格单元世界长度显示到状态栏。
    void updateGridCellLengthStatus(double length_m);

    /// 清空鼠标坐标显示。
    void clearMousePositionStatus();

    /// 连接网络遥测控制面板和通信通道。
    void setupTelemetryPanel(VirtualChannel* channel);

    /// 重置遥测频率显示。
    void resetTelemetryFrequencyDisplay();

    Ui::MainWindow *ui;
    std::unique_ptr<ChannelManager> channelManager_;
    QPushButton* focusMapButton_{nullptr};
    QLabel* mousePositionLabel_{nullptr};
    QLabel* gridCellLengthLabel_{nullptr};

    int m_hideLeftIndex = 0;
    int m_hideRightIndex = 0;
};
#endif // MAINWINDOW_H
