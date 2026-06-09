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
    /// 构造半透明地图悬浮工具条。
    explicit MapOverlayWidget(QWidget* parent = nullptr);

    /// 添加一个带 id 的按钮，点击后通过 buttonClicked 发出 id。
    QPushButton* addButton(const QString& id, const QString& text, const QString& tooltip = QString());

    /// 更新工具条右侧状态文本。
    void setInfoText(const QString& text);

signals:
    /// 工具按钮点击信号，参数为添加按钮时传入的 id。
    void buttonClicked(const QString& id);

private:
    QHBoxLayout* actions_layout_{nullptr};
    QLabel* info_label_{nullptr};
};

}  // namespace map_panel
}  // namespace silverstar

#endif // MAP_OVERLAY_WIDGET_H
