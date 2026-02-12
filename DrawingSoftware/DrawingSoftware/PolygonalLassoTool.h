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

class PolygonalLassoTool : public QWidget
{
    Q_OBJECT
public:
    explicit PolygonalLassoTool(UIManager* ui, QWidget* parent = nullptr);
    QVector<QImage> layers;
    LayerManager* layerManager;
    QImage overlay;

    float zoomPercentage = 100.0;
    QPoint panOffset;

    QVector<QPainterPath> selectionsPath;

    bool isFirstClick = true;
    bool isComplete;
    bool isDrawing;

    void zoomIn();
    void zoomOut();
    void resetZoom();

    void undo();
    void redo();

    void updateSelectionOverlay();
    void clearSelectionOverlay();

    QVector<QPoint> points;
    QStack<bool> isDrawingUndoStack;
    QStack<bool> isDrawingRedoStack;

    QStack<bool> isCompletedUndoStack;
    QStack<bool> isCompletedRedoStack;

    QStack<bool> makingAdditionalSelectionUndoStack;
    QStack<bool> makingAdditionalSelectionRedoStack;

    QStack<bool> makingRemovalUndoStack;
    QStack<bool> makingRemovalRedoStack;

    QStack<bool> isFirstClickUndoStack;
    QStack<bool> isFirstClickRedoStack;

    QStack<QVector<QPoint>> pointsUndoStack;
    QStack<QVector<QPoint>> pointsRedoStack;





signals:
    void polygonalLassoDisabled();

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
    //void updateSelectionOverlay();
    //void clearSelectionOverlay();

    QPainterPath mapPointsOfPolygon(QPolygonF polygon, int numberOfPoints);

private:
    UIManager* uiManager;


    QPoint point;
    QPoint hoverPoint;
    QPainterPath mergedSelectionsPath;
    QPolygon selection;

    QImage pngBackground;
    QImage image;
    QImage background;
    QImage originalImage;
    QImage selectedImage;

    bool makingAdditionalSelection;
    bool makingRemoval;

    QPoint lastPanPoint;
    bool isPanning;
    bool panningEnabled;
};

