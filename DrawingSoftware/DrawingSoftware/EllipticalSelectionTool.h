#pragma once


#include <QWidget>
//#include <QStack>
#include <QImage>
//#include <QVector>
#include <QDir>

#include <QMouseEvent>
#include <QTabletEvent>
#include <QPainter>
#include <QPainterPath>
#include <QVector>
#include <QPoint>
#include <QPolygon>
#include <QColor>

#include <ColourWindow.h>
#include "LayerManager.h"
#include <UIManager.h>

class UIManager;

class EllipticalSelectionTool : public QWidget
{
    Q_OBJECT
public:
    explicit EllipticalSelectionTool(UIManager* ui, QWidget* parent = nullptr);
    QVector<QImage> layers;
    LayerManager* layerManager;
    QImage overlay;

    float zoomPercentage = 100.0;
    QPoint panOffset;

    QVector<QPainterPath> selectionsPath;

    void clearSelectionOverlay();
    void updateSelectionOverlay();

    void zoomIn();
    void zoomOut();
    void resetZoom();

    void undo();
    void redo();

signals:
    void ellipticalSelectionToolDisabled();

    void brushEnabled();

protected:
    void applyZoom(float zoomAmount);

    QPoint mapToImage(const QPoint& p);

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

    void paintEvent(QPaintEvent* event) override;

private:
    UIManager* uiManager;

    QPoint startPoint;
    QPoint hoverPoint;
    QPoint releasePoint;
    QPoint centralPoint;
    bool isDrawing;
    bool isDrawingSquare;
    bool isDrawingInPlace;

    float xDifference;
    float yDifference;

    QPainterPath mergedSelectionsPath;
    QVector<QPoint> points;
    QPolygon selection;

    QImage pngBackground;
    QImage image;
    QImage background;
    QImage originalImage;
    QImage selectedImage;

    bool makingAdditionalSelection;
    bool makingRemoval;
    bool isPanning;

    QPoint lastPanPoint;

    bool panningEnabled;

    QPainterPath mapPointsOfPolygon(QPolygonF polygon, int numberOfPoints);

};