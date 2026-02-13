
#include "LassoTool.h"
#include <LayerManager.h>

#include<HueDial.h>


QPoint LassoTool::mapToImage(const QPoint& p)
{
    QPoint center = rect().center();
    QPoint offsetPoint = (p - center - panOffset) / (zoomPercentage / 100.0);
    return offsetPoint + QPoint(image.width() / 2.0, image.height() / 2.0);
}

LassoTool::LassoTool(UIManager* ui, QWidget* parent)
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

void LassoTool::zoomIn()
{
    applyZoom((1.111));
}

void LassoTool::zoomOut()
{
    applyZoom((0.9));
}

void LassoTool::resetZoom()
{
    zoomPercentage = 100.0;
    applyZoom(1);
}

void LassoTool::applyZoom(float zoomAmount)
{
    if (1 <= zoomPercentage * zoomAmount <= 12800)
    {
        zoomPercentage = zoomPercentage * zoomAmount;
    }
    else if (zoomPercentage < 1) { zoomPercentage = 1; }
    else { zoomPercentage = 12800; }
    update();
}

void LassoTool::undo()
{
    uiManager->undoManager->undo();
    layers = layerManager->layers;
    overlay = layerManager->selectionOverlay;
    selectionsPath = layerManager->selectionsPath;
    update();
}

void LassoTool::redo()
{
    uiManager->undoManager->redo();
    layers = layerManager->layers;
    overlay = layerManager->selectionOverlay;
    selectionsPath = layerManager->selectionsPath;
    update();
}
void LassoTool::keyPressEvent(QKeyEvent* event)
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
        emit lassoDisabled();
        emit brushEnabled();
    }
}

void LassoTool::keyReleaseEvent(QKeyEvent* event)
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

void LassoTool::mousePressEvent(QMouseEvent* event)
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
    }
}

void LassoTool::mouseMoveEvent(QMouseEvent* event)
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
    else {

        points.append(mapToImage(event->pos()));
        updateSelectionOverlay();
    }
    update();

}
void LassoTool::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (isPanning) {
            isPanning = false;
        }
        if (points.length() <= 2)
        {
            points.clear();
            updateSelectionOverlay();

            update();
            uiManager->undoManager->selectionOverlay = overlay;
            uiManager->undoManager->pushUndo(layers);
            uiManager->undoManager->selectionsPath = selectionsPath;

            layerManager->update();
            return;
        }

        points.append(points[0]);
        selection = QPolygon(points);
        QPainterPath newPath = QPainterPath();
        newPath.addPolygon(selection);

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
    points.clear();

    updateSelectionOverlay();
    update();
    uiManager->undoManager->selectionOverlay = overlay;
    uiManager->undoManager->pushUndo(layers);
    uiManager->undoManager->selectionsPath = selectionsPath;

    layerManager->update();
}

void LassoTool::paintEvent(QPaintEvent* event)
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

void LassoTool::clearSelectionOverlay()
{
    overlay.fill(Qt::transparent);
    update();
}

void LassoTool::updateSelectionOverlay()
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
    if (points.count() > 1)
    {
        painter.drawPolyline(QPolygon(points));

    }
    update();
}

