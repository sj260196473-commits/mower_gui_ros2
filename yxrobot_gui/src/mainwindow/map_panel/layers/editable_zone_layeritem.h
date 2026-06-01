#ifndef EDITABLE_ZONE_LAYERITEM_H
#define EDITABLE_ZONE_LAYERITEM_H

#include "mainwindow/map_panel/core/editable_zone_model.h"
#include "mainwindow/map_panel/core/map_layeritem_virtual.h"
#include "common/common.h"
#include "common/map_coordinate_transformer.h"

#include <QRectF>

namespace silverstar {
namespace map_panel {

class EditableZoneLayerItem : public MapLayerBase
{
    Q_OBJECT
public:
    EditableZoneLayerItem(const QString& id, const QString& name, const int& z, QGraphicsItem* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    QRectF boundingRect() const override;

    void setSceneRect(const QRectF& rect);
    void setMapId(const QString& mapId);
    QString mapId() const;
    void updateMap(const OccupancyMap& map);

    void beginDrawing(EditableZoneKind kind);
    bool isDrawing() const;
    bool addWorldPoint(const QPointF& worldPoint);
    bool finishDrawing();
    void cancelDrawing();
    bool selectAtWorldPoint(const QPointF& worldPoint);
    bool removeSelectedZone();
    void clearZones();
    bool selectedZone(EditableZone* zone) const;

    const EditableZoneModel& model() const;
    QString planningZonesJson() const;
    QString blockedAreasJson() const;

signals:
    void zonesChanged();
    void zoneCommitRequested(const QString& planningZonesJson, const QString& blockedAreasJson);

private:
    QColor colorForKind(EditableZoneKind kind, int alpha = 140) const;
    QVector<QPointF> toScenePoints(const QVector<QPointF>& worldPoints) const;
    void drawZone(QPainter* painter, const EditableZone& zone, bool selected);
    QString nextZoneId() const;
    void emitCommitRequested();

private:
    QRectF scene_rect_;
    QString map_id_;
    MapCoordinateTransformer coordinate_transformer_;
    EditableZoneModel model_;
    EditableZoneKind drawing_kind_{EditableZoneKind::NoEntry};
    QVector<QPointF> drawing_points_;
    bool drawing_{false};
    QString selected_zone_id_;
    int next_id_{1};
};

}  // namespace map_panel
}  // namespace silverstar

#endif // EDITABLE_ZONE_LAYERITEM_H
