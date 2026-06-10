#include "mainwindow.h"
#include "ui_mainwindow.h"

using silverstar::map_panel::MapGraphicsView;

/// 构造主窗口并把通信通道绑定到地图视图。
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setupStatusBar();

    channelManager_ = std::make_unique<ChannelManager>();

    auto channel = channelManager_->getChannel();
    if (channel) {
        ui->graphicsView->setCommChannel(channel);

        if (!channel->Init()) {
            std::cerr << "Failed to initialize channel" << std::endl;
            return;
        }
    }
}

/// 销毁 Qt Designer 生成的 UI 对象。
MainWindow::~MainWindow()
{
    delete ui;
}

/// 初始化状态栏按钮和坐标/网格信息显示。
void MainWindow::setupStatusBar()
{
    focusMapButton_ = new QPushButton("Focus Map", this);
    focusMapButton_->setToolTip("Focus map view");
    ui->statusbar->addWidget(focusMapButton_);

    mousePositionLabel_ = new QLabel(this);
    clearMousePositionStatus();
    ui->statusbar->addPermanentWidget(mousePositionLabel_);

    gridCellLengthLabel_ = new QLabel("Grid: -", this);
    ui->statusbar->addPermanentWidget(gridCellLengthLabel_);

    connect(
        ui->graphicsView,
        &MapGraphicsView::mousePositionChanged,
        this,
        &MainWindow::updateMousePositionStatus);

    connect(
        ui->graphicsView,
        &MapGraphicsView::mouseLeftScene,
        this,
        &MainWindow::clearMousePositionStatus);

    connect(
        ui->graphicsView,
        &MapGraphicsView::gridCellLengthChanged,
        this,
        &MainWindow::updateGridCellLengthStatus);

    connect(
        focusMapButton_,
        &QPushButton::clicked,
        ui->graphicsView,
        &MapGraphicsView::focusMapView);
}

/// 根据地图视图发出的坐标信号刷新状态栏文本。
void MainWindow::updateMousePositionStatus(
    const QPointF& scene_pos,
    const QPointF& world_pos,
    bool has_world)
{
    if (!mousePositionLabel_) {
        return;
    }

    if (has_world) {
        mousePositionLabel_->setText(
            QString("Scene: x=%1, y=%2    World: x=%3 m, y=%4 m")
                .arg(scene_pos.x(), 0, 'f', 2)
                .arg(scene_pos.y(), 0, 'f', 2)
                .arg(world_pos.x(), 0, 'f', 3)
                .arg(world_pos.y(), 0, 'f', 3));
        return;
    }

    mousePositionLabel_->setText(
        QString("Scene: x=%1, y=%2    World: -, -")
            .arg(scene_pos.x(), 0, 'f', 2)
            .arg(scene_pos.y(), 0, 'f', 2));
}

/// 鼠标离开地图视图时恢复坐标占位文本。
void MainWindow::clearMousePositionStatus()
{
    if (!mousePositionLabel_) {
        return;
    }

    mousePositionLabel_->setText("Scene: -, -    World: -, -");
}

/// 根据网格长度自动使用米或厘米显示。
void MainWindow::updateGridCellLengthStatus(double length_m)
{
    if (!gridCellLengthLabel_) {
        return;
    }

    if (length_m >= 1.0) {
        gridCellLengthLabel_->setText(QString("Grid: %1 m").arg(length_m, 0, 'f', 2));
        return;
    }

    gridCellLengthLabel_->setText(QString("Grid: %1 cm").arg(length_m * 100.0, 0, 'f', 1));
}

/// 响应右侧隐藏按钮，奇偶点击次数用于切换显示状态。
void MainWindow::on_hide_right_btn_clicked()
{
    m_hideRightIndex++;
    if(m_hideRightIndex%2 == 1)
        ui->dockWidget_3->hide();
    else
        ui->dockWidget_3->show();
}

/// 响应左侧隐藏按钮，奇偶点击次数用于切换显示状态。
void MainWindow::on_hide_left_btn_clicked()
{
    m_hideLeftIndex++;
    if(m_hideLeftIndex%2 == 1)
        ui->dockWidget_2->hide();
    else
        ui->dockWidget_2->show();
}
