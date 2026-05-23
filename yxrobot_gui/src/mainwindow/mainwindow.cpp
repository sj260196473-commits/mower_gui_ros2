#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mousePositionLabel_ = new QLabel("Scene: -, -    World: -, -", this);
    ui->statusbar->addPermanentWidget(mousePositionLabel_);

    connect(
        ui->graphicsView,
        &MapGraphicsView::mousePositionChanged,
        this,
        [this](const QPointF& scene_pos, const QPointF& world_pos, bool has_world) {
            if (has_world) {
                mousePositionLabel_->setText(
                    QString("Scene: x=%1, y=%2    World: x=%3 m, y=%4 m")
                        .arg(scene_pos.x(), 0, 'f', 2)
                        .arg(scene_pos.y(), 0, 'f', 2)
                        .arg(world_pos.x(), 0, 'f', 3)
                        .arg(world_pos.y(), 0, 'f', 3));
            } else {
                mousePositionLabel_->setText(
                    QString("Scene: x=%1, y=%2    World: -, -")
                        .arg(scene_pos.x(), 0, 'f', 2)
                        .arg(scene_pos.y(), 0, 'f', 2));
            }
        });

    connect(
        ui->graphicsView,
        &MapGraphicsView::mouseLeftScene,
        this,
        [this]() {
            mousePositionLabel_->setText("Scene: -, -    World: -, -");
        });

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
