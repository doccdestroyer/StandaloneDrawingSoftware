#include "LayerManager.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <string>

LayerManager::LayerManager(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle("Layers");
    resize(300, 600);
    setMaximumWidth(300);
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);

    layersList = new QListWidget(this);
    layersList->addItems({ "Background", "Layer 1" });

    pngBackground = QImage(QDir::currentPath() + "/Images/PNGBackground.png");

    background = QImage(1100, 1100, QImage::Format_ARGB32_Premultiplied);
    image = background;
    image.fill(Qt::transparent);
    originalImage = image;
    background.fill(Qt::white);
    layers = { background, image };
    selectionOverlay = image;

    auto* layout = new QVBoxLayout(this);

    QPushButton* newLayerButton = new QPushButton("New Layer", this);
    QPushButton* deleteLayerButtton = new QPushButton("Delete", this);


    layout->addWidget(layersList);
    layout->addWidget(newLayerButton);
    layout->addWidget(deleteLayerButtton);

    pushUndo();

    layersList->setCurrentItem(layersList->item(1));


    onLayerClicked(layersList->item(1));
    {
        int index = 1;
        emit layerSelected(layersList->item(1)->text(), index);
    };
    connect(layersList, &QListWidget::itemClicked,
        this, &LayerManager::onLayerClicked);

    connect(newLayerButton, &QPushButton::clicked, this, [=]() {
        int newIndex;
        if (layers.count() == 0)
        {
            layersAdded = 0;
            newIndex = 0;
        }
        else
        {
            layersAdded += 1;
            newIndex = layersList->row(layersList->currentItem()) + 1;

        }
        QString name = QString("Layer %1").arg(layersAdded);
        addLayer(newIndex, name);
        });
    connect(deleteLayerButtton, &QPushButton::clicked,
        this, &LayerManager::onDeleteClicked);
}

void LayerManager::updateLayers(QVector<QImage> newLayers, QImage newOverlay, float newPercentage, QPoint newOffset, QVector<QPainterPath> newSelectionsPath)
{
    layers = newLayers;
    selectionOverlay = newOverlay;
    zoomPercentage = newPercentage;
    panOffset = newOffset;
    selectionsPath = newSelectionsPath;
}

void LayerManager::addLayer(int destination, QString& name)
{
    destination = qBound(0, destination, layersList->count());
    emit layerAdded(destination);
    layersList->insertItem(destination, name);
    layersList->setCurrentItem(layersList->item(destination));
    pushUndo();
    update();
    qDebug() << layersList->count();

}


void LayerManager::undo()
{
    if (undoStack2.size() <= 1) return;
    changingStack = true;

    QStringList current;
    for (int i = 0; i < layersList->count(); ++i)
        current << layersList->item(i)->text();

    redoStack2.push(current);
    undoStack2.pop();
    restore(undoStack2.top());
}

void LayerManager::restore(const QStringList& state)
{
    layersList->clear();
    for (const QString& name : state)
        layersList->addItem(name);
}

void LayerManager::redo()
{
    if (redoStack2.isEmpty()) return;
    changingStack = true;

    QStringList current;
    for (int i = 0; i < layersList->count(); ++i)
        current << layersList->item(i)->text();

    undoStack2.push(current);
    restore(redoStack2.pop());
}

void LayerManager::pushUndo()
{
    QStringList state;
    for (int i = 0; i < layersList->count(); ++i)
        state << layersList->item(i)->text();

    undoStack2.push(state);
    redoStack2.clear();
    changingStack = false;
}

void LayerManager::onLayerClicked(QListWidgetItem* item)
{
    if (!item) return;
    layer = item->text();
    int index = layersList->row(item);
    emit layerSelected(layer, index);
}

void LayerManager::onDeleteClicked()
{
    int row = layersList->currentRow();
    if (layersList->count() < 1 || row < 0) return;

    QListWidgetItem* item = layersList->takeItem(row);
    delete item;

    if (layersList->count() <= 1) {
        layersList->setCurrentRow(0);
        qDebug() << layer;
    }

    int newNum = row - 1;
    if (newNum == -1) {
        newNum = 0;
    }

    emit layerDeleted(row);
    pushUndo();
    update();
}