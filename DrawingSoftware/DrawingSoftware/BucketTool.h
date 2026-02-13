#pragma once

#include <QWidget>
#include <QStack>
#include <QImage>
#include <QVector>
#include <QDir>
#include <QMouseEvent>
#include <QTabletEvent>
#include <QPainter>
#include <QColor>

#include <ColourWindow.h>
#include "LayerManager.h"
#include <UIManager.h>


class BucketTool : public QWidget
{
    Q_OBJECT

public:
    explicit BucketTool(UIManager* ui, QWidget* parent = nullptr);
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void undo();
    void redo();
    LayerManager* layerManager;
    ColourWindow* colourWindow;

    QVector<QImage> layers;
    QImage overlay;

    float zoomPercentage = 100.0;
    QPoint panOffset;

    QVector<QPainterPath> selectionsPath;

    UIManager* uiManager;

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void applyZoom(float zoomAmount);

    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

    QImage fill(QImage& image, int startX, int startY, const QColor& oldColor, const QColor& newColor);

signals:
    void lassoEnabled();
    void brushEnabled();

    void bucketDisabled();

private:
    bool panningEnabled = false;
    bool isPanning = false;
    QPoint lastPanPoint;

    bool inSelection;

    QColor newColour = QColor(Qt::red);

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
    bool isErasing = false;
    bool isDrawing = false;
    int delayCounter;

    QPoint lastPoint;
    QPointF lastPointF;

    QImage image;
    QImage background;
    QImage originalImage;
    QImage selectedImage;

    QStack<QVector<QImage>> undoStack;
    QStack<QVector<QImage>> redoStack;


    QColor colour;

    QPoint mapToImage(const QPoint& p);
    QPointF mapToImageF(const QPointF& p);

    QImage adjustBrushColour(const QImage& brush, const QColor& color);
};