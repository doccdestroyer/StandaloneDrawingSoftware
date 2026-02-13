
#include <EllipticalSelectionTool.h>
#include <LayerManager.h>



#include<HueDial.h>


QPoint EllipticalSelectionTool::mapToImage(const QPoint& p)
{
    QPoint center = rect().center();
    QPoint offsetPoint = (p - center - panOffset) / (zoomPercentage / 100.0);
    return offsetPoint + QPoint(image.width() / 2.0, image.height() / 2.0);
}

EllipticalSelectionTool::EllipticalSelectionTool(UIManager* ui, QWidget* parent)
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
    background.fill(Qt::white);
    overlay = QImage(image.size(), QImage::Format_ARGB32_Premultiplied);
    overlay.fill(Qt::transparent);

    uiManager = ui;
    layerManager = uiManager->undoManager->layerManager;
    overlay = layerManager->selectionOverlay;
    layers = layerManager->layers;
    layerManager->layers = layers;
    layerManager->update();
}

void EllipticalSelectionTool::zoomIn()
{
    applyZoom((1.111));
}

void EllipticalSelectionTool::zoomOut()
{
    applyZoom((0.9));
}

void EllipticalSelectionTool::resetZoom()
{
    zoomPercentage = 100.0;
    applyZoom(1);
}

void EllipticalSelectionTool::applyZoom(float zoomAmount)
{
    if (1 <= zoomPercentage * zoomAmount <= 12800)
    {
        zoomPercentage = zoomPercentage * zoomAmount;
    }
    else if (zoomPercentage < 1) { zoomPercentage = 1; }
    else { zoomPercentage = 12800; }
    update();
}

void EllipticalSelectionTool::undo()
{
    uiManager->undoManager->undo();
    layers = layerManager->layers;
    overlay = layerManager->selectionOverlay;
    selectionsPath = layerManager->selectionsPath;
    update();
}

void EllipticalSelectionTool::redo()
{
    uiManager->undoManager->redo();
    layers = layerManager->layers;
    overlay = layerManager->selectionOverlay;
    selectionsPath = layerManager->selectionsPath;
    update();
}

void EllipticalSelectionTool::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == 61 && event->modifiers() & Qt::ControlModifier)
    {
        zoomIn();
    }
    if (event->key() == 45 && event->modifiers() & Qt::ControlModifier)
    {
        zoomOut();
    }
    if (event->key() == Qt::Key_0 && event->modifiers() & Qt::ControlModifier)
    {
        resetZoom();
    }
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
    if (event->key() == Qt::Key_B)
    {
        emit ellipticalSelectionToolDisabled();
        emit brushEnabled();
    }
}
void EllipticalSelectionTool::keyReleaseEvent(QKeyEvent* event)
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

void EllipticalSelectionTool::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (panningEnabled)
        {
            lastPanPoint = event->pos();
            isPanning = true;
        }
        else
        {
            if (event->modifiers() & Qt::AltModifier)
            {
                makingAdditionalSelection = false;
                makingRemoval = true;
            }
            else if (event->modifiers() & Qt::ShiftModifier)
            {
                makingAdditionalSelection = true;
                makingRemoval = false;
            }

            else
            {
                makingAdditionalSelection = false;
                makingRemoval = false;
                selectionsPath.clear();
                mergedSelectionsPath.clear();
                clearSelectionOverlay();
            }
            update();
        }
        releasePoint = QPoint(0, 0);
        startPoint = mapToImage(event->pos());
        isDrawing = true;
        updateSelectionOverlay();
    }
}

void EllipticalSelectionTool::mouseMoveEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton)) return;
    if (isPanning)
    {
        if (!lastPanPoint.isNull())
        {
            QPoint change = event->position().toPoint() - lastPanPoint;
            panOffset += change;
            lastPanPoint = event->position().toPoint();
        }
    }
    else if (isDrawing) {
        if (event->modifiers() & Qt::AltModifier)
        {
            isDrawingInPlace = true;
        }
        else
        {
            isDrawingInPlace = false;
        }
        if (event->modifiers() & Qt::ShiftModifier)
        {
            isDrawingSquare = true;
        }
        else
        {
            isDrawingSquare = false;
        }

        hoverPoint = mapToImage(event->pos());
        updateSelectionOverlay();
    }
    update();

}
void EllipticalSelectionTool::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (isPanning) {
            isPanning = false;
        }
        else
        {
            if (isDrawing)
            {
                isDrawing = false;
                releasePoint = mapToImage(event->pos());
            }
            if (isDrawingSquare)
            {
                isDrawingSquare = false;
                xDifference = (releasePoint.x() - startPoint.x());
                yDifference = (releasePoint.y() - startPoint.y());
                float variance = std::min(abs(xDifference), abs(yDifference));

                int directionX;
                int directionY;
                if (xDifference < 0)
                {
                    directionX = -1;
                }
                else
                {
                    directionX = 1;
                }
                if (yDifference < 0)
                {
                    directionY = -1;
                }
                else
                {
                    directionY = 1;
                }
                releasePoint.setX(startPoint.x() + variance * directionX);
                releasePoint.setY(startPoint.y() + variance * directionY);
                updateSelectionOverlay();
            }
            else
            {
                releasePoint = mapToImage(event->pos());
                isDrawing = false;
                update();
            }
            if (isDrawingInPlace)
            {
                isDrawingInPlace = false;
                centralPoint = startPoint;
                startPoint = hoverPoint;
                xDifference = (hoverPoint.x() - centralPoint.x());
                yDifference = (hoverPoint.y() - centralPoint.y());
                releasePoint.setX(centralPoint.x() - xDifference);
                releasePoint.setY(centralPoint.y() - yDifference);
            }
            else if (!isDrawingSquare)
            {
                update();
            }

            QRect ellipseRectangle = QRect(startPoint, releasePoint);
            QPainterPath ellipsePath;
            ellipsePath.addEllipse(ellipseRectangle);
            QPolygonF ellipsePolygonF = ellipsePath.toFillPolygon();

            QPolygonF newPolygonF = QPolygonF(mapPointsOfPolygon(ellipsePolygonF, 100).toFillPolygon());
            QPainterPath newPath;
            newPath.addPolygon(newPolygonF);


            if (makingRemoval)
            {
                bool removedFromMerge = false;


                for (int i = 0; i < selectionsPath.length(); ++i)
                {
                    QPainterPath& path = selectionsPath[i];
                    if (path.intersects(newPath))
                    {
                        QPainterPath subtractionPath = path.subtracted(newPath);
                        selectionsPath[i] = subtractionPath;
                        removedFromMerge = true;
                        bool changed = true;
                        while (changed)
                        {
                            changed = false;
                            for (int k = 0; k < selectionsPath.length(); ++k)
                            {
                                QPainterPath& otherPath = selectionsPath[k];
                                if (i == k) continue;

                                if (selectionsPath[i].intersects(otherPath))
                                {
                                    selectionsPath[i] = selectionsPath[i].subtracted(otherPath);
                                    selectionsPath.erase(selectionsPath.begin() + k);
                                    changed = true;
                                    break;
                                }
                            }
                        }
                    }
                    if (!removedFromMerge)
                    {
                        selectionsPath.append(newPath);
                    }

                }

            }
            else if (makingAdditionalSelection)
            {
                bool mergedAnyPolygons = false;
                for (int i = 0; i < selectionsPath.length(); ++i)
                {
                    QPainterPath& path = selectionsPath[i];

                    if (path.intersects(newPath)) {
                        QPainterPath mergedPath = path.united(newPath);
                        selectionsPath[i] = mergedPath;

                        mergedAnyPolygons = true;
                        bool changed = true;

                        while (changed)
                        {
                            changed = true;
                            for (int j = 0; j < selectionsPath.length(); ++j)
                            {
                                QPainterPath& otherPath = selectionsPath[j];
                                if (j == i) continue;

                                if (selectionsPath[i].intersects(otherPath))
                                {
                                    selectionsPath[i] = selectionsPath[i].united(otherPath);
                                    selectionsPath.erase(selectionsPath.begin() + j);
                                    changed = true;
                                    break;
                                }

                            }
                            break;
                        }
                    }
                    if (!mergedAnyPolygons)
                    {
                        selectionsPath.append(newPath);
                    }
                }
            }
            else
            {
                {
                    selectionsPath.clear();
                    selectionsPath.append(newPath);
                }
            }
        }

    }

    clearSelectionOverlay();
    updateSelectionOverlay();
    update();
    uiManager->undoManager->selectionOverlay = overlay;
    uiManager->undoManager->pushUndo(layers);
    uiManager->undoManager->selectionsPath = selectionsPath;

    layerManager->selectionsPath = selectionsPath;
    layerManager->selectionOverlay = overlay;

    layerManager->update();
}

QPainterPath EllipticalSelectionTool::mapPointsOfPolygon(QPolygonF polygon, int numberOfPoints)
{
    QPainterPath path;
    path.addPolygon(polygon);

    for (int i = 0; i < numberOfPoints; i++)
    {
        path.pointAtPercent(i / numberOfPoints - 1);
    }
    return path;
}


void EllipticalSelectionTool::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    QPoint center = rect().center();

    painter.translate(center);
    painter.scale(zoomPercentage / 100, zoomPercentage / 100);
    painter.translate(panOffset / (zoomPercentage / 100.0));

    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    QPointF topLeft(-image.width() / 2.0, -image.height() / 2.0);
    painter.fillRect(rect(), Qt::black);

    painter.drawImage(topLeft, pngBackground);

    for (const QImage layer : layers) {
        painter.drawImage(topLeft, layer);
    }
    painter.drawImage(topLeft, overlay);
}

void EllipticalSelectionTool::clearSelectionOverlay()
{
    overlay.fill(Qt::transparent);
    update();
}

void EllipticalSelectionTool::updateSelectionOverlay()
{

    overlay.fill(Qt::transparent);
    QPainter painter(&overlay);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    QPoint center = image.rect().center();

    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    QPointF topLeft(center);

    QPen outlinePen = QPen(Qt::red, 1);
    QBrush fillBrush = QBrush(QColor(255, 0, 0, 50));

    painter.setPen(outlinePen);
    painter.setBrush(fillBrush);

    for (const QPainterPath& path : selectionsPath) {
        QVector<QPolygonF> allPolys = path.toFillPolygons();
        for (const QPolygonF& polyF : allPolys) {
            QPolygon polyQ;
            for (const QPointF& p : polyF) {
                polyQ << QPoint(
                    static_cast<int>(std::round(p.x())),
                    static_cast<int>(std::round(p.y()))
                );
            }
            painter.drawPolygon(polyQ);
        }
    }

    QPen hoverPen = QPen(Qt::red, 1, Qt::DashLine);
    QBrush hoverBrush = QBrush(QColor(Qt::transparent));

    painter.setPen(hoverPen);
    painter.setBrush(hoverBrush);


    if (startPoint != QPoint(0, 0) && hoverPoint != QPoint(0, 0) && isDrawing)
    {
        if (isDrawingSquare)
        {
            xDifference = hoverPoint.x() - startPoint.x();
            yDifference = hoverPoint.y() - startPoint.y();
            float variance = std::min(abs(xDifference), abs(yDifference));

            int directionX;
            int directionY;
            if (xDifference < 0)
            {
                directionX = -1;
            }
            else
            {
                directionX = 1;
            }
            if (yDifference < 0)
            {
                directionY = -1;
            }
            else
            {
                directionY = 1;
            }
            hoverPoint.setX(startPoint.x() + variance * directionX);
            hoverPoint.setY(startPoint.y() + variance * directionY);

            QRect ellipseRect = QRect(startPoint, hoverPoint);

            if (!isDrawingInPlace)
            {
                painter.drawEllipse(ellipseRect);
            }
            update();
        }
        if (isDrawingInPlace)
        {
            centralPoint = startPoint;
            QPoint initialPoint = hoverPoint;
            xDifference = initialPoint.x() - centralPoint.x();
            yDifference = initialPoint.y() - centralPoint.y();

            QPoint temporaryReleasePoint;
            temporaryReleasePoint.setX(centralPoint.x() - xDifference);
            temporaryReleasePoint.setY(centralPoint.y() - yDifference);

            QRect ellipseRect = QRect(initialPoint, temporaryReleasePoint);
            painter.drawEllipse(ellipseRect);
        }
        else if (isDrawing && !isDrawingSquare)
        {
            QRect ellipseRect = QRect(startPoint, hoverPoint);
            painter.drawEllipse(ellipseRect);
        }
        if (!isDrawing)
        {
            clearSelectionOverlay();
        }
    }
    update();
}

