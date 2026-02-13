#pragma once

#include <QWidget>
#include <QListWidget>
#include <QStack>
#include <QDir>
#include <QPainterPath>

class LayerManager : public QWidget
{
    Q_OBJECT

public:
    explicit LayerManager(QWidget* parent = nullptr);
    void addLayer(int destination, QString& name);
    void undo();
    void redo();
    void pushUndo();
    void updateList();
    void restore(const QStringList& state);
    QListWidget* layersList = nullptr;

    float zoomPercentage = 100.0;
    QPoint panOffset;
    QVector<QPainterPath> selectionsPath;

    QImage pngBackground;
    QImage background;
    QImage image;
    QImage originalImage;
    QVector<QImage> layers;
    QImage selectionOverlay;

    void updateLayers(QVector<QImage> newLayers, QImage newOverlay, float newPercentage, QPoint newOffset, QVector<QPainterPath> newSelectionsPath);


signals:
    void layerSelected(const QString& layerName, int& index);
    void layerDeleted(int& index);
    void layerAdded(const int& index);

private slots:
    void onLayerClicked(QListWidgetItem* item);
    void onDeleteClicked();


private:

    int layersAdded = 1;

    QString layer;

    QStack<QListWidget*> undoStack;
    QStack<QListWidget*> redoStack;

    using LayerState = QStringList;

    QStack<LayerState> undoStack2;
    QStack<LayerState> redoStack2;

    bool changingStack;
};