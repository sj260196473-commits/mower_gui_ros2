#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPointF>
#include "channel/channel_manager.h"
#include "common/common.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_hide_right_btn_clicked();
    void on_hide_left_btn_clicked();

private:
    void setupStatusBar();
    void updateMousePositionStatus(const QPointF& scene_pos, const QPointF& world_pos, bool has_world);
    void clearMousePositionStatus();

    Ui::MainWindow *ui;
    std::unique_ptr<ChannelManager> channelManager_;
    QLabel* mousePositionLabel_{nullptr};

    int m_hideLeftIndex = 0;
    int m_hideRightIndex = 0;
};
#endif // MAINWINDOW_H
