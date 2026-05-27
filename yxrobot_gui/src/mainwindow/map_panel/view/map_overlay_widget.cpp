#include "mainwindow/map_panel/view/map_overlay_widget.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSizePolicy>

namespace silverstar {
namespace map_panel {

MapOverlayWidget::MapOverlayWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("mapOverlayWidget");
    setAttribute(Qt::WA_StyledBackground, true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFixedHeight(42);

    info_label_ = new QLabel("Overlay: ready", this);
    info_label_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    auto* separator = new QFrame(this);
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Sunken);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 6, 10, 6);
    layout->setSpacing(8);
    actions_layout_ = new QHBoxLayout();
    actions_layout_->setContentsMargins(0, 0, 0, 0);
    actions_layout_->setSpacing(8);
    layout->addLayout(actions_layout_);
    layout->addWidget(separator);
    layout->addWidget(info_label_, 1);

    setStyleSheet(
        "#mapOverlayWidget {"
        "  background-color: rgba(24, 24, 24, 220);"
        "  border: 1px solid rgba(255, 255, 255, 45);"
        "  border-radius: 6px;"
        "}"
        "#mapOverlayWidget QLabel {"
        "  color: #e6e6e6;"
        "}"
        "#mapOverlayWidget QPushButton {"
        "  background-color: #3f3f3f;"
        "  color: #eeeeee;"
        "  border: 1px solid #666666;"
        "  border-radius: 4px;"
        "  padding: 3px 10px;"
        "}"
        "#mapOverlayWidget QPushButton:hover {"
        "  background-color: #4a4a4a;"
        "}"
        "#mapOverlayWidget QPushButton:pressed {"
        "  background-color: #333333;"
        "}");

    addButton("overlay.action", "Action", "Overlay action");
}

QPushButton* MapOverlayWidget::addButton(const QString& id, const QString& text, const QString& tooltip)
{
    auto* button = new QPushButton(text, this);
    if (!tooltip.isEmpty()) {
        button->setToolTip(tooltip);
    }

    actions_layout_->addWidget(button);
    connect(button, &QPushButton::clicked, this, [this, id]() {
        emit buttonClicked(id);
    });
    return button;
}

void MapOverlayWidget::setInfoText(const QString& text)
{
    info_label_->setText(text);
}

}  // namespace map_panel
}  // namespace silverstar
