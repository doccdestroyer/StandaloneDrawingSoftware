#pragma once
#pragma once

#include <LayerManager.h>

#include <QWidget>
#include <QStack>
#include <QVector>
#include <QImage>

class UndoManager : public QWidget
{
public:
	LayerManager* layerManager;

	explicit UndoManager(QWidget* parent = nullptr);
	void undo();
	void redo();
	void pushUndo(const QVector<QImage>& layers);

	QStack<QVector<QImage>> undoLayerStack;
	QStack<QVector<QImage>> redoLayerStack;

	QStack<QImage> undoSelectionStack;
	QStack<QImage> redoSelectionStack;

	QStack<QVector<QPainterPath>> undoSelectionPathStack;
	QStack<QVector<QPainterPath>> redoSelectionPathStack;

	QVector<QPainterPath> selectionsPath;
	QImage selectionOverlay;

private:
	QImage image;
	QImage background;
	QImage originalImage;
	QVector<QImage> layers;
};

