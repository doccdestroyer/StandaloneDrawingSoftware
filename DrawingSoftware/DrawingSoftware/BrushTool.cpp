#include "BrushTool.h"
#include <LayerManager.h>
#include<HueDial.h>

BrushTool::BrushTool(UIManager* ui, QWidget* parent)
    : QWidget(parent), uiManager(ui)
{
    setAttribute(Qt::WA_TabletTracking);
    setAttribute(Qt::WA_MouseTracking);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    pngBackground = QImage(QDir::currentPath() + "/Images/PNGBackground.png");
    background = QImage(1100, 1100, QImage::Format_ARGB32_Premultiplied);
    image = background;
    image.fill(Qt::transparent);
    originalImage = image;

    brush = QImage(QDir::currentPath() + "/Images/ChalkRot.png");
    brushOutline = QImage(QDir::currentPath() + "/Images/ChalkRot_Outline.png");

    colourWindow = ui->colourWindow;
    uiManager = ui;
    layerManager = uiManager->undoManager->layerManager;
    overlay = layerManager->selectionOverlay;
    layers = layerManager->layers;
    uiManager->undoManager->undoLayerStack.push(layers);
    layerManager->layers = layers;
    layerManager->update();

    connect(colourWindow, &ColourWindow::colourChanged,
        this, [=](QColor newColor)
        {
            colour = newColor;
        });

    connect(layerManager, &LayerManager::layerSelected,
        this, [=](const QString& layerName, int layerIndex) {
            selectedLayerIndex = layerIndex;
        });


    connect(layerManager, &LayerManager::layerAdded,
        this, [=](int layerIndex) {

            // Add layer to layers (shared variable) and update local variables accoridingly

            layers.insert(layerIndex, originalImage);
            selectedLayerIndex = layerIndex;
            layerManager->layers = layers;

            uiManager->undoManager->pushUndo(layers);
            uiManager->undoManager->selectionOverlay = overlay;
            uiManager->undoManager->selectionsPath = selectionsPath;

            uiManager->undoManager->pushUndo(layers);
            layerManager->pushUndo();

            uiManager->undoManager->redoSelectionStack.clear();
            uiManager->undoManager->redoLayerStack.clear();
            uiManager->undoManager->redoSelectionPathStack.clear();

            layerManager->update();
            update();
        });


    connect(layerManager, &LayerManager::layerDeleted,
        this, [=](int layerIndex) {
            // Delete layer from layers (shared variable) and update local variables accoridingly
            if (layerIndex < 0 || layerIndex >= layers.size()) {
                return;
            }
            layers.removeAt(layerIndex);
            layerManager->layers = layers;

            uiManager->undoManager->selectionOverlay = overlay;
            uiManager->undoManager->selectionsPath = selectionsPath;
            uiManager->undoManager->pushUndo(layers);

            layerManager->pushUndo();
            uiManager->undoManager->redoSelectionStack.clear();
            uiManager->undoManager->redoLayerStack.clear();
            uiManager->undoManager->redoSelectionPathStack.clear();

            if (layers.count() == 0) {
                selectedLayerIndex = 0;
            }
            else
            {
                selectedLayerIndex = layerIndex - 1;
            }
            layerManager->update();
            update();
        });



}

void BrushTool::tabletEvent(QTabletEvent* event)
{
    if (event->type() == QEvent::TabletPress) {
        if (layers.count() < 1) return;
        if (panningEnabled) {
            lastPanPoint = event->position().toPoint();
            isPanning = true;
        }
        else {
            drawing = true;
            usingTablet = true;
            lastPointF = mapToImageF(getScaledPointF(event->position()));
            brush = adjustBrushColour(brush, colour);
        }
    }

    if (event->type() == QEvent::TabletMove) {
        if (isPanning)
        {

            if (!lastPanPoint.isNull()) {
                QPoint change = event->position().toPoint() - lastPanPoint;
                panOffset += change;
                lastPanPoint = event->position().toPoint();
                update();

            }
        }
        else if (drawing)
        {
            if (layers.count() < 1) return; // If there are no layers, do not draw

            currentPoint = mapToImageF(getScaledPointF(event->position()));
            qreal pressure = event->pressure();

            if (tiltEnabled)
            {
                xTilt = event->xTilt();
                yTilt = event->yTilt();
            }
            else
            {
                xTilt = 0;
                yTilt = 0;
            }

            QPainter painter(&layers[selectedLayerIndex]);

            painter.setRenderHint(QPainter::Antialiasing, true);
            if (isErasing) {
                painter.setCompositionMode(QPainter::CompositionMode_Clear);

            }
            drawStroke(painter, lastPointF, currentPoint, pressure);
            lastPointF = currentPoint;

            hoverPoint = event->position();
            isHovering = true;
            // Delay on hover to keep lag minimal
            delayCounter += 1;
            if (delayCounter == 5) {
                delayCounter = 0;

                update();
            }
        }
        else {
            if (tiltEnabled)
            {
                xTilt = event->xTilt();
                yTilt = event->yTilt();
            }
            else
            {
                xTilt = 0;
                yTilt = 0;
            }
            // Delay on hover to keep lag minimal
            hoverPoint = event->position();
            isHovering = true;
            delayCounter += 1;
            if (delayCounter == 5) {
                delayCounter = 0;
                update();

            }
        }

    }
    if (event->type() == QEvent::TabletRelease)
    {
        if (isPanning) {
            isPanning = false;
        }
        if (layers.count() < 1) return;

        lastPointF = currentPoint;
        drawing = false;
        usingTablet = true;

        // push Undo

        uiManager->undoManager->pushUndo(layers);
        layerManager->layers = layers;
        layerManager->pushUndo();
        uiManager->undoManager->selectionOverlay = overlay;
        uiManager->undoManager->selectionsPath = selectionsPath;

        uiManager->undoManager->redoSelectionStack.clear();
        uiManager->undoManager->redoLayerStack.clear();
        uiManager->undoManager->redoSelectionPathStack.clear();

        layerManager->update();
        update();
    }
    event->accept();
}
void BrushTool::undo()
{
    if (layerManager->layersList->count() == 0) return;
    uiManager->undoManager->undo();
    layers = layerManager->layers;

    // Update selected layer if applicable
    if (selectedLayerIndex >= layerManager->layers.count())
    {
        selectedLayerIndex -= 1;
    }
    else if (selectedLayerIndex < 0)
    {
        selectedLayerIndex = 0;
    }
    update();
}

void BrushTool::redo()
{
    uiManager->undoManager->redo();
    layers = layerManager->layers;
    update();
}

void BrushTool::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == 61 && event->modifiers() & Qt::ControlModifier)
        zoomIn();
    if (event->key() == 45 && event->modifiers() & Qt::ControlModifier)
        zoomOut();

    if (event->key() == Qt::Key_0 && event->modifiers() & Qt::ControlModifier)
        resetZoom();

    if (event->key() == Qt::Key_Z && event->modifiers() & Qt::ControlModifier)
    {
        undo();
    }
    if (event->key() == Qt::Key_Y && event->modifiers() & Qt::ControlModifier)
    {
        redo();
    }
    if (event->key() == Qt::Key_Space)
    {
        panningEnabled = true;
        setCursor(Qt::OpenHandCursor);
    }
    if (event->key() == Qt::Key_E)
    {
        if (isErasing == true) {
            isErasing = false;
        }
        else {

            isErasing = true;
        }
    }
    if (event->key() == Qt::Key_L)
    {
        emit brushDisabled();
        emit lassoEnabled();
    }
    if (event->key() == Qt::Key_K)
    {
        emit brushDisabled();
        emit bucketEnabled();
    }
    if (event->key() == 91)
    {
        alterBrushSize(-1);
    }
    if (event->key() == 93)
    {
        alterBrushSize(1);
    }
    if (event->key() == 91)
    {
        alterBrushSize(-1);
    }
    if (event->key() == 93) {
        alterBrushSize(1);
    }
}

void BrushTool::keyReleaseEvent(QKeyEvent* event)
{
    if (event->isAutoRepeat())
    {
        return;
    }
    if (event->key() == Qt::Key_Space)
    {
        lastPanPoint = QPoint();
        panningEnabled = false;
        setCursor(Qt::ArrowCursor);
    }
}
void BrushTool::applyZoom(float zoomAmount)
{
    // Bounds for zoom percentage
    if (1 <= zoomPercentage * zoomAmount <= 12800)
    {
        zoomPercentage = zoomPercentage * zoomAmount;
    }
    else if (zoomPercentage < 1) { zoomPercentage = 1; }
    else { zoomPercentage = 12800; }
    update();
}

void BrushTool::zoomIn()
{
    applyZoom((1.111));
}

void BrushTool::zoomOut()
{
    applyZoom((0.9));
}

void BrushTool::resetZoom()
{
    zoomPercentage = 100.0;
    applyZoom(1);
}

QPoint BrushTool::getScaledPoint(QPoint pos) {
    return pos;
}

QPointF BrushTool::getScaledPointF(QPointF pos) {
    return pos;
}

void BrushTool::alterBrushSize(int multiplier)
{
    int alterAmoumt;
    if (brushSize < 10) {
        alterAmoumt = 1;
    }
    else if (brushSize < 50) {
        alterAmoumt = 5;
    }
    else if (brushSize < 100) {
        alterAmoumt = 10;
    }
    else if (brushSize < 200) {
        alterAmoumt = 25;
    }
    else {
        alterAmoumt = 100;
    }

    brushSize = brushSize + alterAmoumt * multiplier;
    brushSize = qBound(1, brushSize, 500);
    update();

}
void BrushTool::mousePressEvent(QMouseEvent* event)
{
    layers = layerManager->layers;
    if (usingTablet == true) return;
    if (layers.count() < 1) return;

    selectionsPath = layerManager->selectionsPath;

    if (event->button() == Qt::LeftButton) {
        if (panningEnabled) {
            lastPanPoint = event->pos();
            isPanning = true;
        }
        else {
            drawing = true;
            brush = adjustBrushColour(brush, colour);
        }
    }
}

void BrushTool::mouseMoveEvent(QMouseEvent* event)
{

    if (usingTablet == true) return; // Do not register mouse if using tablet
    if (layers.count() < 1) return;
    if (isPanning)
    {
        if (!lastPanPoint.isNull()) {
            QPoint change = event->pos() - lastPanPoint;
            panOffset += change;
            lastPanPoint = event->pos();
        }
    }
    else
    {
        if (drawing && (event->buttons() & Qt::LeftButton)) {
            QPoint currentPoint = mapToImage(getScaledPoint(event->pos()));
            QPainter painter(&layers[selectedLayerIndex]);
            painter.setRenderHint(QPainter::Antialiasing, true);
            if (isErasing) {
                painter.setCompositionMode(QPainter::CompositionMode_Clear);
            }
            drawStroke(painter, lastPoint.toPointF(), currentPoint.toPointF(), 1.0);
            lastPoint = currentPoint;
            update();
        }

        hoverPoint = event->pos();
        isHovering = true;
        if (delayCounter == 5) {
            repaint();
            delayCounter = 0;
        }
    }
    layerManager->layers[selectedLayerIndex] = layers[selectedLayerIndex];
    layerManager->layers = layers;
    layerManager->update();
    update();
}

void BrushTool::mouseReleaseEvent(QMouseEvent* event)
{
    if (layers.count() < 1) return;
    inSelection = false;
    if (isPanning) {
        isPanning = false;
    }

    if (usingTablet == true) return;

    if (event->button() == Qt::LeftButton) {
        drawing = false;

        // Push Undo
        uiManager->undoManager->pushUndo(layers);
        layerManager->layers = layers;
        layerManager->pushUndo();
        uiManager->undoManager->selectionOverlay = overlay;
        //uiManager->undoManager->pushUndo(layers);
        uiManager->undoManager->selectionsPath = selectionsPath;

        uiManager->undoManager->redoSelectionStack.clear();
        uiManager->undoManager->redoLayerStack.clear();
        uiManager->undoManager->redoSelectionPathStack.clear();

        update();
    }

}

void BrushTool::paintEvent(QPaintEvent* event)
{
    overlay = layerManager->selectionOverlay;

    QPainter painter(this);
    QPoint center = rect().center();
    QPoint hoverOffset = center - hoverPoint.toPoint();

    // Translate based on zoom and pan offset
    painter.translate(center);
    painter.scale(zoomPercentage / 100, zoomPercentage / 100);
    painter.translate(panOffset / (zoomPercentage / 100.0));

    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    QPointF topLeft(-image.width() / 2.0, -image.height() / 2.0);

    // Draw background
    painter.drawImage(topLeft, pngBackground);

    // Draw Layers
    for (const QImage layer : layers) {
        painter.drawImage(topLeft, layer);
    }
    // Draw Selection
    painter.drawImage(topLeft, overlay);

    // Undo scale ranslate and offset
    painter.scale(100 / zoomPercentage, 100 / zoomPercentage);
    painter.translate(-center);

    if (isErasing) {
        painter.setCompositionMode(QPainter::CompositionMode_Clear);

    }
    if (isHovering) if (!panningEnabled) {
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        qreal size = brushSize;
        QImage brushHover = brushOutline.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        QTransform t;
        t.rotate(xTilt * 3);
        t.scale(zoomPercentage / 100.0, zoomPercentage / 100.0);
        brushHover = brushHover.transformed(t, Qt::SmoothTransformation);

        QPointF drawPos(hoverPoint.x() - brushHover.width() / 2, hoverPoint.y() - brushHover.height() / 2);
        drawPos = center - hoverOffset - panOffset;
        drawPos = QPoint(drawPos.x() - brushHover.width() / 2, drawPos.y() - brushHover.height() / 2);

        painter.drawImage(drawPos, brushHover);
    }
}






void BrushTool::removeLayer(int layer)
{
    // Remove selected layer -> get layer back to possible bounds and if possible go to the layer below
    selectedLayerIndex -= 1;
    if (selectedLayerIndex == layer)
    {
        selectedLayerIndex = qMax(0, selectedLayerIndex - 1);
    }
    else if (selectedLayerIndex > layer)
    {
        selectedLayerIndex--;
    }
}

QPoint BrushTool::mapToImage(const QPoint& p)
{
    QPoint center = rect().center();
    QPoint offsetPoint = (p - center - panOffset) / (zoomPercentage / 100.0);
    return offsetPoint + QPoint(image.width() / 2.0, image.height() / 2.0);
}

QPointF BrushTool::mapToImageF(const QPointF& p)
{
    QPointF center = rect().center();
    QPointF offsetPoint = (p - center - panOffset) / (zoomPercentage / 100.0);
    return offsetPoint + QPointF(image.width() / 2.0, image.height() / 2.0);
}


void BrushTool::drawStroke(QPainter& p, const QPointF& from, const QPointF& to, qreal pressure)
{
    QLineF line(from, to);
    qreal dist = line.length();

    // Draw brush 4 times equidistantly if distance is greater than 2.5
    if (dist > 2.5 && spacing < 2.5) {
        QPointF quaterPoint = lastPointF + QPointF((lastPointF.x() - currentPoint.x()) / 4, (lastPointF.y() - currentPoint.y()) / 4);
        drawBrush(p, quaterPoint, pressure);
        QPointF midPoint = lastPointF + QPointF((lastPointF.x() - currentPoint.x()) / 2, (lastPointF.y() - currentPoint.y()) / 2);
        drawBrush(p, midPoint, pressure);
        QPointF threeQuaterPoint = lastPointF + QPointF((lastPointF.x() - currentPoint.x()) / 4 * 3, (lastPointF.y() - currentPoint.y()) / 4 * 3);
        drawBrush(p, threeQuaterPoint, pressure);
    }

    if (dist > spacing || spacing != 0.0) {
        drawBrush(p, to, pressure);
        lastPointF = currentPoint;
        return;
    }
}



void BrushTool::drawBrush(QPainter& painter, const QPointF& pos, qreal pressure)
{
    qreal AdjustedBrushSize = brushSize;
    if (pressureAffectsSize)
    {
        AdjustedBrushSize = brushSize * pressure;
    }
    if (!pressureAffectsOpacity)
    {
        pressure = 1; // set pressure to 1 so multiplcation calculations arent affects
    }

    // Scale brush and adjust to tilt rotation
    QImage scaled = brush.scaled(AdjustedBrushSize, AdjustedBrushSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QTransform transform;
    transform.rotate(xTilt * 3);
    scaled = scaled.transformed(transform, Qt::SmoothTransformation);

    if (isErasing) {
        painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    }
    QPointF drawPos(pos.x() - scaled.width() / 2, pos.y() - scaled.height() / 2);
    painter.setOpacity(pressure * opacity);

    QImage temporarylayer = originalImage;
    temporarylayer.fill(Qt::transparent);

    if (selectionsPath.length() > 0)
    {
        QVector<QPainterPath> newPath = selectionsPath;
        QPolygonF imagePolygon = QPolygon(image.rect());
        QPainterPath imagePath;
        imagePath.addPolygon(imagePolygon);

        // Create clip path from all paths in selectionPaths to see if brush is within selection
        bool changed = false;
        for (int i = 0; i < selectionsPath.length(); ++i)
        {

            QPainterPath& path = newPath[i];
            QPainterPath subtractionPath = imagePath.subtracted(path);
            newPath[i] = subtractionPath;

            newPath[i] = (subtractionPath != path) ? subtractionPath : path;
            changed = (subtractionPath != path);
            changed = true;
            while (changed)
            {
                changed = false;
                for (int k = 0; k < newPath.length(); ++k)
                {
                    QPainterPath& otherPath = newPath[k];
                    if (k == i) continue;
                    if (newPath[i].intersects(otherPath))
                    {
                        QPainterPath newSubtraction = newPath[i].subtracted(otherPath);
                        if (newSubtraction != newPath[i]) {
                            newPath[i] = newSubtraction;
                            changed = true;
                        }
                    }
                }
            }
        }
        QPainterPath clipPath = imagePath.subtracted(newPath[0]);
        painter.setClipPath(clipPath);
    }
    painter.drawImage(drawPos, scaled);
}

QImage BrushTool::adjustBrushColour(const QImage& brush, const QColor& color)
{
    if (brush.isNull())
        return QImage(); // Safety check

    QImage coloured = brush.convertToFormat(QImage::Format_ARGB32);

    QPainter painter(&coloured);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(coloured.rect(), colour);
    painter.end();
    return coloured;
};