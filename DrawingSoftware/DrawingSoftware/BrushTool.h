#pragma once

#include <QWidget>
#include <QStack>
#include <QImage>
#include <QVector>
#include <QDir>
#include <QMouseEvent>
#include <QTabletEvent>
#include <QPainter>

#include <ColourWindow.h>
#include "LayerManager.h"
#include <UIManager.h>

class UIManager;

class BrushTool : public QWidget
{
    Q_OBJECT

public:
    explicit BrushTool(UIManager* ui, QWidget* parent = nullptr);
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void undo();
    void redo();
    LayerManager* layerManager;
    ColourWindow* colourWindow;


    void ReturnColour(UIManager& ui);

    QVector<QImage> layers;
    QImage overlay;

    float zoomPercentage = 100.0;
    QPoint panOffset;
    QVector<QPainterPath> selectionsPath;

    qreal  opacity = 1.0;
    bool isErasing = false;
    bool pressureAffectsSize = true;
    bool pressureAffectsOpacity = false;
    bool tiltEnabled = true;
    bool scatteringEnabled = false;
    int brushSize = 50;
    qreal spacing = 0;

    QImage brush;
    QImage brushOutline;
    std::string brushType;

protected:
    void tabletEvent(QTabletEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void applyZoom(float zoomAmount);
    QPoint getScaledPoint(QPoint pos);
    QPointF getScaledPointF(QPointF pos);
    void alterBrushSize(int multiplier);
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

signals:
    void brushDisabled();

    void lassoEnabled();
    void bucketEnabled();

private:

    UIManager* uiManager;
    QPainter painter;
    bool panningEnabled = false;
    bool isPanning = false;
    QPoint lastPanPoint;

    bool inSelection;

    int selectedLayerIndex = 1;
    QImage pngBackground;
    bool isHovering;
    QPointF currentPoint;
    QPointF hoverPoint;
    QPointF pan;

    qreal xTilt;
    qreal yTilt;
    bool drawing = false;
    bool usingTablet = false;
    bool isDrawing = false;
    int delayCounter;

    QColor colour = QColor(Qt::red);

    QPoint lastPoint;
    QPointF lastPointF;

    QImage image;
    QImage background;
    QImage originalImage;
    QImage selectedImage;

    QStack<QVector<QImage>> undoStack;
    QStack<QVector<QImage>> redoStack;

    void removeLayer(int layer);
    void pushUndo(const QVector<QImage>& layers);
    QPoint mapToImage(const QPoint& p);
    QPointF mapToImageF(const QPointF& p);

    void drawStroke(QPainter& p, const QPointF& from, const QPointF& to, qreal pressure);
    void drawBrush(QPainter& p, const QPointF& pos, qreal pressure);
    QImage adjustBrushColour(const QImage& brush, const QColor& color);
};