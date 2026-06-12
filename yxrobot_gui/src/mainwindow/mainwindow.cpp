#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <cstdlib>

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
        setupTelemetryPanel(channel);

        if (!channel->Init()) {
            std::cerr << "Failed to initialize channel" << std::endl;
            return;
        }
    } else {
        ui->connect_btn->setEnabled(false);
        ui->connect_status->setText("通道不可用");
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

void MainWindow::setupTelemetryPanel(VirtualChannel* channel)
{
    const char* env_host = std::getenv("YX_TELEMETRY_HOST");
    if (env_host != nullptr && env_host[0] != '\0') {
        ui->robotIp_LineEdit->setText(QString::fromUtf8(env_host));
    }

    ui->connect_status->setText("未连接");
    ui->connect_btn->setText("连接");
    resetTelemetryFrequencyDisplay();

    connect(ui->connect_btn, &QPushButton::clicked, this, [this, channel]() {
        if (channel->IsTelemetryConnected()) {
            channel->DisconnectTelemetry();
            ui->connect_btn->setText("连接");
            ui->connect_status->setText("未连接");
            resetTelemetryFrequencyDisplay();
            return;
        }

        const QString host = ui->robotIp_LineEdit->text().trimmed();
        if (host.isEmpty()) {
            ui->connect_status->setText("IP为空");
            return;
        }

        if (!channel->SetTelemetryTarget(host, 11086)) {
            ui->connect_status->setText("IP无效");
            return;
        }

        ui->connect_status->setText("连接中");
        ui->connect_btn->setText("断开");
        channel->ConnectTelemetry();
    });

    connect(channel, &VirtualChannel::emitTelemetryConnectionChanged, this,
            [this](bool connected, const QString& message) {
        ui->connect_status->setText(message);
        ui->connect_btn->setText(connected ? "断开" : "连接");
        if (!connected) {
            resetTelemetryFrequencyDisplay();
        }
    });

    connect(channel, &VirtualChannel::emitTelemetryFrequencyChanged, this,
            [this](const QString& topic, double hz) {
        const QString text = QString::number(hz, 'f', 1);
        if (topic == QStringLiteral("pose")) {
            ui->pose_freq->setText(text);
        } else if (topic == QStringLiteral("map")) {
            ui->globalMap_freq->setText(text);
        } else if (topic == QStringLiteral("lidar")) {
            ui->lidar_freq->setText(text);
        }
    });
}

void MainWindow::resetTelemetryFrequencyDisplay()
{
    ui->pose_freq->setText("0");
    ui->globalMap_freq->setText("0");
    ui->lidar_freq->setText("0");
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
