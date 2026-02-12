#include "BrushTool.h"
#include <LayerManager.h>



#include<HueDial.h>


BrushTool::BrushTool(UIManager* ui, QWidget* parent)
    : QWidget(parent), uiManager(ui)
{
    //setWindowTitle("Bad Apple");
    //setFixedSize(1920, 1080);
    //setStyleSheet("background-color: rgb(30,30,30);");
    setAttribute(Qt::WA_TabletTracking);
    setAttribute(Qt::WA_MouseTracking);
    setMouseTracking(true);
    //QStack<QVector<QImage>> undoLayerStack;
    //QStack<QVector<QImage>> redoLayerStack;
    setFocusPolicy(Qt::StrongFocus);


    pngBackground = QImage(QDir::currentPath() + "/Images/PNGBackground.png");

    background = QImage(1100, 1100, QImage::Format_ARGB32_Premultiplied);
    image = background;
    image.fill(Qt::transparent);
    originalImage = image;
    //background.fill(Qt::white);
    //layers = { background, image };

    brush = QImage(QDir::currentPath() + "/Images/ChalkRot.png");

    brushOutline = QImage(QDir::currentPath() + "/Images/ChalkRot_Outline.png");

    //uiManager = new UIManager(this);

    colourWindow = ui->colourWindow;
    //colourWindow->show();




    uiManager = ui;
    //undoStack.push(layers);


    layerManager = uiManager->undoManager->layerManager;

    overlay = layerManager->selectionOverlay;

    layers = layerManager->layers;


    //layerManager->show();
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


            layers.insert(layerIndex, originalImage);
            selectedLayerIndex = layerIndex;
            layerManager->layers = layers;

            uiManager->undoManager->pushUndo(layers);
            uiManager->undoManager->selectionOverlay = overlay;
            uiManager->undoManager->selectionsPath = selectionsPath;




            //
            // 
            uiManager->undoManager->pushUndo(layers);
            layerManager->pushUndo();
            //uiManager->undoManager->pushUndo(layers);

            uiManager->undoManager->redoSelectionStack.clear();
            uiManager->undoManager->redoLayerStack.clear();
            uiManager->undoManager->redoSelectionPathStack.clear();

            layerManager->update();
            update();
        });


    connect(layerManager, &LayerManager::layerDeleted,
        this, [=](int layerIndex) {
            if (layerIndex < 0 || layerIndex >= layers.size()) {
                return;
            }
            layers.removeAt(layerIndex);
            //pushUndo(layers);

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


void BrushTool::ReturnColour(UIManager& ui) {
    //colour = ui.colourWindow->updateColour();
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

            if (!isErasing)
            {
                // FIX ME
                //colour = colourWindow->updateColour();
                //ReturnColour(*uiManager);
                //colour = QColor::fromHsl(0, 0, 0);


            }

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
            if (layers.count() < 1) return;

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

            //if (pressureAffectsOpacity)
            //{
            //    drawStroke(painter, lastPointF, currentPoint, pressure);
            //}
            //else
            //{
            //    drawStroke(painter, lastPointF, currentPoint, 1);
            //}
            drawStroke(painter, lastPointF, currentPoint, pressure);


            lastPointF = currentPoint;

            hoverPoint = event->position();
            isHovering = true;
            delayCounter += 1;
            if (delayCounter == 5) {
                delayCounter = 0;

                update();
            }
        }
        else {
            hoverPoint = event->position();
            isHovering = true;
            delayCounter += 1;
            if (delayCounter == 5) {
                delayCounter = 0;
                update();

            }
        }

    }

    //if (event->type() == QEvent::TabletMove && drawing && !isPanning) {

    //}

    if (event->type() == QEvent::TabletRelease)
    {
        if (isPanning) {
            isPanning = false;
        }
        if (layers.count() < 1) return;

        lastPointF = currentPoint;
        drawing = false;
        usingTablet = true;
        //pushUndo(layers);

        uiManager->undoManager->pushUndo(layers);
        layerManager->layers = layers;
        layerManager->pushUndo();
        uiManager->undoManager->selectionOverlay = overlay;
        //uiManager->undoManager->pushUndo(layers);
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


    if (event->key() == Qt::Key_B)
    {
        if (brushType == "chalk") {

            brush = QImage(QDir::currentPath() + "/Images/CircleBrush.png");
            brushOutline = QImage(QDir::currentPath() + "/Images/CircleBrush_Outline.png");
            brushType = "circle";
        }
        else {
            brush = QImage(QDir::currentPath() + "/Images/ChalkRot.png");
            brushOutline = QImage(QDir::currentPath() + "/Images/ChalkRot_Outline.png");
            brushType = "chalk";

        }
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

    if (1 <= zoomPercentage * zoomAmount <= 12800)
    {
        zoomPercentage = zoomPercentage * zoomAmount;
    }
    else if (zoomPercentage < 1) { zoomPercentage = 1; }
    else { zoomPercentage = 12800; }
    //QPointF hoverFromCenter = rect().center() + hoverPoint;
    repaint();
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
    //return  QPoint(((pos.x() - panOffset.x())), ((pos.y() - panOffset.y())));
    return pos;
}

QPointF BrushTool::getScaledPointF(QPointF pos) {
    //return QPointF(((pos.x() - panOffset.x())), ((pos.y() - panOffset.y())));
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
            if (!isErasing)
            {
                // FIX ME 
                //colour = QColor::fromHsl(0, 0, 0);
                //colour = colourWindow->updateColour();
                //ReturnColour(*uiManager);

            }
            brush = adjustBrushColour(brush, colour);
        }

    }
}

void BrushTool::mouseMoveEvent(QMouseEvent* event)
{

    if (usingTablet == true) return;
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

            if (painter.isActive())
            {
                qDebug() << "PAinterACtive";
            }
            qDebug() << "TOTAL LAYERS: " << layers.count() << "INDEX: " << selectedLayerIndex;

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

    painter.translate(center);
    painter.scale(zoomPercentage / 100, zoomPercentage / 100);
    painter.translate(panOffset / (zoomPercentage / 100.0));

    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    QPointF topLeft(-image.width() / 2.0, -image.height() / 2.0);

    painter.drawImage(topLeft, pngBackground);

    for (const QImage layer : layers) {
        painter.drawImage(topLeft, layer);
    }
    painter.drawImage(topLeft, overlay);

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

void BrushTool::pushUndo(const QVector<QImage>& layers)
{
    //undoStack.push(layers);
    //if (undoStack.size() > 50) undoStack.remove(0);
}

QPoint BrushTool::mapToImage(const QPoint& p)
{
    QPoint center = rect().center();
    QPoint offsetPoint = (p - center - panOffset) / (zoomPercentage / 100.0);
    //QPoint offsetPoint = (p / (zoomPercentage / 100.0) - center - panOffset / (zoomPercentage / 100.0));

    return offsetPoint + QPoint(image.width() / 2.0, image.height() / 2.0);
}

QPointF BrushTool::mapToImageF(const QPointF& p)
{
    QPointF center = rect().center();
    QPointF offsetPoint = (p - center - panOffset) / (zoomPercentage / 100.0);
    //QPoint offsetPoint = (p / (zoomPercentage / 100.0) - center - panOffset / (zoomPercentage / 100.0)).toPoint();

    return offsetPoint + QPointF(image.width() / 2.0, image.height() / 2.0);
}


void BrushTool::drawStroke(QPainter& p, const QPointF& from, const QPointF& to, qreal pressure)
{
    QLineF line(from, to);
    qreal dist = line.length();

    //if (dist > 2.5 && spacing < 2.5) {
    //    QPointF quaterPoint = lastPointF + QPointF((lastPointF.x() - currentPoint.x()) / 4, (lastPointF.y() - currentPoint.y()) / 4);
    //    drawBrush(p, quaterPoint, pressure);
    //    QPointF midPoint = lastPointF + QPointF((lastPointF.x() - currentPoint.x()) / 2, (lastPointF.y() - currentPoint.y()) / 2);
    //    drawBrush(p, midPoint, pressure);
    //    QPointF threeQuaterPoint = lastPointF + QPointF((lastPointF.x() - currentPoint.x()) / 4 * 3, (lastPointF.y() - currentPoint.y()) / 4 * 3);
    //    drawBrush(p, threeQuaterPoint, pressure);
    //}

    if (dist > spacing || spacing != 0.0) {
        drawBrush(p, to, pressure);
        lastPointF = currentPoint;
        return;
    }

    //p.end();

}



void BrushTool::drawBrush(QPainter& p, const QPointF& pos, qreal pressure)
{
    qreal size = brushSize;
    if (pressureAffectsSize)
    {
        qDebug() << pressure;
        size = brushSize * pressure;
    }


    if (!pressureAffectsOpacity)
    {
        pressure = 1;
    }

    QImage scaled = brush.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QTransform t;
    t.rotate(xTilt * 3);


    scaled = scaled.transformed(t, Qt::SmoothTransformation);

    if (isErasing) {
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    }
    QPointF drawPos(pos.x() - scaled.width() / 2, pos.y() - scaled.height() / 2);
    p.setOpacity(pressure * opacity);
    //p.drawImage(drawPos, scaled);

    QImage temporarylayer = originalImage;
    temporarylayer.fill(Qt::transparent);

    //p.end();
    //p = (&temporarylayer);
    if (selectionsPath.length() > 0)
    {
        //p.drawImage(drawPos, scaled);

        QVector<QPainterPath> newPath = selectionsPath;
        QPolygonF imagePolygon = QPolygon(image.rect());
        QPainterPath imagePath;
        imagePath.addPolygon(imagePolygon);


        bool changed = false;
        for (int i = 0; i < selectionsPath.length(); ++i)
        {

            QPainterPath& path = newPath[i];
            QPainterPath subtractionPath = imagePath.subtracted(path);
            newPath[i] = subtractionPath;

            newPath[i] = (subtractionPath != path) ? subtractionPath : path;  // One-liner fix here
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
        p.setClipPath(clipPath);
    }
    p.drawImage(drawPos, scaled);
    //p.end();
}

QImage BrushTool::adjustBrushColour(const QImage& brush, const QColor& color)
{
    if (brush.isNull())
        return QImage();

    QImage coloured = brush.convertToFormat(QImage::Format_ARGB32);

    QPainter p(&coloured);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(coloured.rect(), colour);
    p.end();




    return coloured;
};





