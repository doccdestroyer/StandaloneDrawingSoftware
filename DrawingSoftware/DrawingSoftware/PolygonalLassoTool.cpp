
#include <PolygonalLassoTool.h>
#include <LayerManager.h>
//x #include<HueDial.h>


QPoint PolygonalLassoTool::mapToImage(const QPoint& p)
{
    QPoint center = rect().center();
    QPoint offsetPoint = (p - center - panOffset) / (zoomPercentage / 100.0);
    return offsetPoint + QPoint(image.width() / 2.0, image.height() / 2.0);
}

PolygonalLassoTool::PolygonalLassoTool(UIManager* ui, QWidget* parent)
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

    isComplete = false;
    makingAdditionalSelection = false;
    makingRemoval = false;
    isFirstClickUndoStack.push(isFirstClick);
    isCompletedUndoStack.push(isComplete);
    isDrawingUndoStack.push(isDrawing);
    pointsUndoStack.push(points);
    makingRemovalUndoStack.push(makingRemoval);
    makingAdditionalSelectionUndoStack.push(makingAdditionalSelection);
}

void PolygonalLassoTool::zoomIn()
{
    applyZoom((1.111));
}

void PolygonalLassoTool::zoomOut()
{
    applyZoom((0.9));
}

void PolygonalLassoTool::resetZoom()
{
    zoomPercentage = 100.0;
    applyZoom(1);
}

void PolygonalLassoTool::applyZoom(float zoomAmount)
{
    if (1 <= zoomPercentage * zoomAmount <= 12800)
    {
        zoomPercentage = zoomPercentage * zoomAmount;
    }
    else if (zoomPercentage < 1) { zoomPercentage = 1; }
    else { zoomPercentage = 12800; }
    update();
}

void PolygonalLassoTool::undo()
{
    // Check if can undo
    if (isCompletedUndoStack.size() <= 1) return;
    if (isDrawingUndoStack.size() <= 1) return;
    if (makingAdditionalSelectionUndoStack.size() <= 1) return;
    if (pointsUndoStack.size() <= 1) return;
    if (makingRemovalUndoStack.size() <= 1) return;
    if (isFirstClickUndoStack.size() <= 1) return;

    // Undo shared and local variables
    uiManager->undoManager->undo();
    layers = layerManager->layers;
    overlay = layerManager->selectionOverlay;
    selectionsPath = layerManager->selectionsPath;

    isFirstClickRedoStack.push(isFirstClickUndoStack.pop());
    isFirstClick = isFirstClickUndoStack.top();

    isCompletedRedoStack.push(isCompletedUndoStack.pop());
    isComplete = isCompletedUndoStack.top();

    isDrawingRedoStack.push(isDrawingUndoStack.pop());
    isDrawing = isDrawingUndoStack.top();

    makingRemovalRedoStack.push(makingRemovalUndoStack.pop());
    makingRemoval = makingRemovalUndoStack.top();

    makingAdditionalSelectionRedoStack.push(makingAdditionalSelectionUndoStack.pop());
    makingAdditionalSelection = makingAdditionalSelectionUndoStack.top();

    pointsRedoStack.push(pointsUndoStack.pop());
    points = pointsUndoStack.top();

    updateSelectionOverlay();
    overlay = layerManager->selectionOverlay;
    update();
}

void PolygonalLassoTool::redo()
{
    // Check if can redo
    if (isFirstClickRedoStack.isEmpty()) return;
    if (isCompletedRedoStack.isEmpty()) return;
    if (isDrawingRedoStack.isEmpty()) return;
    if (makingAdditionalSelectionRedoStack.isEmpty()) return;
    if (pointsRedoStack.isEmpty()) return;

    // Redo shared and local variables
    uiManager->undoManager->redo();
    layers = layerManager->layers;
    overlay = layerManager->selectionOverlay;
    selectionsPath = layerManager->selectionsPath;

    isFirstClick = isFirstClickRedoStack.pop();
    isFirstClickUndoStack.push(isFirstClick);

    isComplete = isCompletedRedoStack.pop();
    isCompletedUndoStack.push(isComplete);

    isDrawing = isDrawingRedoStack.pop();
    isDrawingUndoStack.push(isDrawing);

    makingAdditionalSelection = makingAdditionalSelectionRedoStack.pop();
    makingAdditionalSelectionUndoStack.push(makingAdditionalSelection);

    points = pointsRedoStack.pop();
    pointsUndoStack.push(points);
    update();
}
void PolygonalLassoTool::keyPressEvent(QKeyEvent* event)
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
        emit polygonalLassoDisabled();
        emit brushEnabled();
    }
}

void PolygonalLassoTool::keyReleaseEvent(QKeyEvent* event)
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

QPainterPath PolygonalLassoTool::mapPointsOfPolygon(QPolygonF polygon, int numberOfPoints)
{
    QPainterPath path;
    path.addPolygon(polygon);

    // Separate points on polygon into the desired amount of points
    for (int i = 0; i < numberOfPoints; i++)
    {
        path.pointAtPercent(i / numberOfPoints - 1);
    }
    return path;
}

void PolygonalLassoTool::mousePressEvent(QMouseEvent* event)
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
            point = mapToImage(event->pos());

            /////// FIRST CLICK LOGIC
            if (isFirstClick) {
            
                isComplete = false;
                points.clear();
                points.append(point);
                isDrawing = true;
                isFirstClick = false;

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
            /////// MULTIPLE CLICKS LOGIC
            else
            {
                // Check if recent point is close enough to first point to finish selection
                if ((point - points[0]).manhattanLength() < 20)
                {
                    isComplete = true;
                    isFirstClick = true;
                    points.append(points[0]);
                    isDrawing = false;
                    if (!makingAdditionalSelection)
                    {
                        selection.clear();

                    }
                    selection.append(QPolygon(points));
                    QPolygonF newPolygon = QPolygonF(QPolygon(points));
                    QPainterPath polygonPath;
                    polygonPath.addPolygon(newPolygon);
                    newPolygon = polygonPath.toFillPolygon();
                    QPolygonF newPolygonF = QPolygonF(mapPointsOfPolygon(newPolygon, 100).toFillPolygon());
                    QPainterPath newPath;
                    newPath.addPolygon(newPolygonF);

                    // If not making additonal or removal, clear previous polygons and add the new one
                    if (!makingRemoval && !makingAdditionalSelection && isComplete)
                    {
                        selectionsPath.clear();
                        selectionsPath.append(newPath);
                    }
                    else if (isComplete)
                    {
                        if (makingRemoval)
                        {
                            // Removal logic
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
                        // Additional selection logic
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


                    }
                    points.clear();
                }
                else
                {
                    points.append(point);

                }
            }
            // Push Undos
            pointsUndoStack.push(points);
            isFirstClickUndoStack.push(isFirstClick);

            isCompletedUndoStack.push(isComplete);
            isDrawingUndoStack.push(isDrawing);

            makingRemovalUndoStack.push(makingRemoval);
            makingAdditionalSelectionUndoStack.push(makingAdditionalSelection);

            layerManager->update();

        }
        updateSelectionOverlay();
        
        uiManager->undoManager->selectionOverlay = overlay;
        uiManager->undoManager->pushUndo(layers);
        uiManager->undoManager->selectionsPath = selectionsPath;

        layerManager->update();
        update();
    }
}

void PolygonalLassoTool::mouseMoveEvent(QMouseEvent* event)
{
    if (isPanning && event->button() == Qt::LeftButton)
    {
        if (!lastPanPoint.isNull())
        {
            // Update offset based on change in movement
            QPoint change = event->position().toPoint() - lastPanPoint;
            panOffset += change;
            lastPanPoint = event->position().toPoint();
        }
    }
    else
    {
        hoverPoint = mapToImage(event->pos());
        if (isDrawing)
            updateSelectionOverlay();
    }
    update();

}
void PolygonalLassoTool::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (isPanning) {
            isPanning = false;
        }
    }
    update();
}

void PolygonalLassoTool::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    QPoint center = rect().center();

    // Translate painter to take into account zoom and pan offset
    painter.translate(center);
    painter.scale(zoomPercentage / 100, zoomPercentage / 100);
    painter.translate(panOffset / (zoomPercentage / 100.0));

    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    QPointF topLeft(-image.width() / 2.0, -image.height() / 2.0);
    painter.fillRect(rect(), Qt::black);
    
    // Draw background
    painter.drawImage(topLeft, pngBackground);

    // Draw layers
    for (const QImage layer : layers) {
        painter.drawImage(topLeft, layer);
    }
    // Draw overlay
    painter.drawImage(topLeft, overlay);
}

void PolygonalLassoTool::clearSelectionOverlay()
{
    overlay.fill(Qt::transparent);
    update();
}

void PolygonalLassoTool::updateSelectionOverlay()
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

    // Draw all selections
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
    if (points.count() > 1)
    {
        painter.drawPolyline(QPolygon(points));

    }
    if (!hoverPoint.isNull() && points.count() > 0 && !isComplete && isDrawing && !isFirstClick)
    {
        painter.setPen(QPen(Qt::red, 1, Qt::DashLine));
        painter.drawLine(points[points.count()-1], hoverPoint);
    }
    update();
}