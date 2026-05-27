#include "mainwindow.h"
#include "ui_mainwindow.h"

using silverstar::map_panel::MapGraphicsView;

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

MainWindow::~MainWindow()
{
    delete ui;
}

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

void MainWindow::clearMousePositionStatus()
{
    if (!mousePositionLabel_) {
        return;
    }

    mousePositionLabel_->setText("Scene: -, -    World: -, -");
}

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

void MainWindow::on_hide_right_btn_clicked()
{
    m_hideRightIndex++;
    if(m_hideRightIndex%2 == 1)
        ui->dockWidget_3->hide();
    else
        ui->dockWidget_3->show();
}

void MainWindow::on_hide_left_btn_clicked()
{
    m_hideLeftIndex++;
    if(m_hideLeftIndex%2 == 1)
        ui->dockWidget_2->hide();
    else
        ui->dockWidget_2->show();
}
