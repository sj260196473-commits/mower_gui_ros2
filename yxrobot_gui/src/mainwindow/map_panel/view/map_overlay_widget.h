#ifndef MAP_OVERLAY_WIDGET_H
#define MAP_OVERLAY_WIDGET_H

#include <QWidget>

class QLabel;
class QPushButton;
class QHBoxLayout;

namespace silverstar {
namespace map_panel {

class MapOverlayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MapOverlayWidget(QWidget* parent = nullptr);

    QPushButton* addButton(const QString& id, const QString& text, const QString& tooltip = QString());
    void setInfoText(const QString& text);

signals:
    void buttonClicked(const QString& id);

private:
    QHBoxLayout* actions_layout_{nullptr};
    QLabel* info_label_{nullptr};
};

}  // namespace map_panel
}  // namespace silverstar

#endif // MAP_OVERLAY_WIDGET_H
