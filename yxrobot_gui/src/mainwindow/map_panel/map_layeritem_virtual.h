#ifndef MAP_LAYER_BASE_H
#define MAP_LAYER_BASE_H
#include <QGraphicsObject>

namespace silverstar {
namespace map_panel {

class MapLayerBase : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit MapLayerBase(const QString& layerId,
                          const QString& layerName,
                          const QString& layerGroup,
                          QGraphicsItem* parent = nullptr);

    QString getLayerId() const{return m_layerId;}
    QString getLayerName() const{return m_layerName;}
    QString getLayerGroup() const{return m_layerGroup;}

private:
    QString m_layerId;
    QString m_layerGroup;
    QString m_layerName;
};

}  // namespace map_panel
}  // namespace silverstar

#endif // MAP_LAYER_BASE_H
