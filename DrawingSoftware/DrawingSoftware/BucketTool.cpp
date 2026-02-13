#include "BucketTool.h"
#include "BrushTool.h"



#include<HueDial.h>



BucketTool::BucketTool(UIManager* ui, QWidget* parent)
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

    colourWindow = ui->colourWindow;
    uiManager = ui;

    layerManager = uiManager->undoManager->layerManager;
    layers = layerManager->layers;

    overlay = layerManager->selectionOverlay;

    connect(colourWindow, &ColourWindow::colourChanged,
        this, [=](QColor newColor)
        {
            colour = newColor;
        });


    connect(layerManager, &LayerManager::layerSelected,
        this, [=](const QString& layerName, int layerIndex) {
            selectedLayerIndex = layerIndex;
            update();
        });


    connect(layerManager, &LayerManager::layerAdded,
        this, [=](int layerIndex) {
            selectedLayerIndex = layerIndex;
            update();
        });
}


void BucketTool::applyZoom(float zoomAmount)
{

    if (1 <= zoomPercentage * zoomAmount <= 12800)
    {
        zoomPercentage = zoomPercentage * zoomAmount;
    }
    else if (zoomPercentage < 1) { zoomPercentage = 1; }
    else { zoomPercentage = 12800; }
    update();
}

void BucketTool::zoomIn()
{
    applyZoom((1.111));
}

void BucketTool::zoomOut()
{
    applyZoom((0.9));
}

void BucketTool::resetZoom()
{
    zoomPercentage = 100.0;
    applyZoom(1);
}

void BucketTool::undo()
{
    // Undo from manager and update local variables accordingly
    uiManager->undoManager->undo();
    layers = layerManager->layers;
    overlay = layerManager->selectionOverlay;
    selectionsPath = layerManager->selectionsPath;

    layerManager->update();
    update();
}

void BucketTool::redo()
{
    // Redo from manager and update local variables accordingly
    uiManager->undoManager->redo();
    layers = layerManager->layers;
    overlay = layerManager->selectionOverlay;
    selectionsPath = layerManager->selectionsPath;
    update();
}
void BucketTool::keyPressEvent(QKeyEvent* event)
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

    if (event->key() == Qt::Key_L)
    {
        emit bucketDisabled();
        emit lassoEnabled();
    }

    if (event->key() == Qt::Key_B)
    {
        emit bucketDisabled();
        emit brushEnabled();
    }
}

void BucketTool::keyReleaseEvent(QKeyEvent* event)
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

void BucketTool::paintEvent(QPaintEvent* event)
{
    overlay = uiManager->undoManager->selectionOverlay;


    QPainter painter(this);
    QPoint center = rect().center();

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
}


QPoint BucketTool::mapToImage(const QPoint& p)
{
    QPoint center = rect().center();
    QPoint offsetPoint = (p - center - panOffset) / (zoomPercentage / 100.0);
    return offsetPoint + QPoint(image.width() / 2.0, image.height() / 2.0);
}

QPointF BucketTool::mapToImageF(const QPointF& p)
{
    QPointF center = rect().center();
    QPointF offsetPoint = (p - center - panOffset) / (zoomPercentage / 100.0);
    return offsetPoint + QPointF(image.width() / 2.0, image.height() / 2.0);
}

void BucketTool::mousePressEvent(QMouseEvent* event)
{
    if (layers.count() < 1) return;

    if (event->button() == Qt::LeftButton) {
        if (panningEnabled) {
            lastPanPoint = event->pos();
            isPanning = true;
        }
        else
        {
            QPoint point = mapToImage(event->pos());

            selectionsPath = uiManager->undoManager->selectionsPath;
            if (selectionsPath.length() > 0)
            {
                QVector<QPainterPath> newPath = selectionsPath;
                QPolygonF imagePolygon = QPolygon(image.rect());
                QPainterPath imagePath;
                imagePath.addPolygon(imagePolygon);

                // Merge selection paths to get a clip path for the bucket
                bool changed = false;
                for (int i = 0; i < selectionsPath.length(); ++i)
                {

                    QPainterPath& path = newPath[i];
                    QPainterPath subtractionPath = imagePath.subtracted(path);

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

                if (clipPath.contains(point)) // If path contains ppoint, fill and update layer, else, do nothing
                {
                    QImage currentImage = layers[selectedLayerIndex];
                    QColor basePixelColor = QColor(currentImage.pixel(point.x(), point.y()));

                    layers[selectedLayerIndex] = fill(currentImage, point.x(), point.y(), basePixelColor, colour);

                }
            }
            else // if no selection, fill and update layer
            {
                QImage currentImage = layers[selectedLayerIndex];
                QColor basePixelColor = QColor(currentImage.pixel(point.x(), point.y()));
                layers[selectedLayerIndex] = fill(currentImage, point.x(), point.y(), basePixelColor, colour);
            }

        }
    }
    // Update local and shared variables
    layerManager->layers[selectedLayerIndex] = layers[selectedLayerIndex];
    layerManager->layers = layers;
    layerManager->update();
    uiManager->undoManager->pushUndo(layers);
    uiManager->undoManager->selectionsPath = selectionsPath;

    uiManager->undoManager->redoLayerStack.clear();
    uiManager->undoManager->redoSelectionPathStack.clear();
    uiManager->undoManager->redoSelectionStack.clear();

    update();
}
void BucketTool::mouseMoveEvent(QMouseEvent* event)
{
    if (isPanning)
    {
        if (!lastPanPoint.isNull()) {
            QPoint change = event->position().toPoint() - lastPanPoint;
            panOffset += change;
            lastPanPoint = event->position().toPoint();
            update();

        }
    }
}

void BucketTool::mouseReleaseEvent(QMouseEvent* event)
{
    if (isPanning) {
        isPanning = false;
    }
}

QImage BucketTool::fill(QImage& image, int startX, int startY,
    const QColor& oldColor, const QColor& newColor)
{
    const QRgb target = image.pixel(startX, startY);
    const QRgb replacement = newColor.rgba();

    // If pixel is the colour, return
    if (target == replacement)
        return image;

    QStack<QPoint> stack;
    stack.push(QPoint(startX, startY));

    // While there are pixels to be completed
    while (!stack.isEmpty())
    {
        QPoint p = stack.pop();
        int x = p.x();
        int y = p.y();

        // Break if pixel value exceeds image dimenstions
        if (x < 0 || x >= image.width() ||
            y < 0 || y >= image.height())
        {
            continue;
        }

        // Break if pixel is starting pixel
        if (image.pixel(x, y) != target)
        {
            continue;
        }

        // If selections, check if point currently being questioned is in the selection
        if (selectionsPath.length() > 0)
        {
            for (int pathNum = 0; pathNum < selectionsPath.count(); pathNum++)
            {
                if (selectionsPath[pathNum].contains(QPoint(x, y)))
                {
                    // Change pixel colour
                    image.setPixel(x, y, replacement);

                    // Expand points
                    stack.push(QPoint(x + 1, y));
                    stack.push(QPoint(x - 1, y));
                    stack.push(QPoint(x, y + 1));
                    stack.push(QPoint(x, y - 1));
                }
            }
        }
        else
        {
            // Change pixel colour
            image.setPixel(x, y, replacement);

            // Expand points
            stack.push(QPoint(x + 1, y));
            stack.push(QPoint(x - 1, y));
            stack.push(QPoint(x, y + 1));
            stack.push(QPoint(x, y - 1));
        }

    }
    return image;
}