#include <UndoManager.h>
#include <LayerManager.h>

UndoManager::UndoManager(QWidget* parent)
    : QWidget(parent)
{
    layerManager = new LayerManager(this);

    selectionOverlay = layerManager->selectionOverlay;
    selectionsPath = layerManager->selectionsPath;
    layers = layerManager->layers;

    undoLayerStack.push(layers);
    undoSelectionPathStack.push(selectionsPath);
    undoSelectionStack.push(selectionOverlay);
}

void UndoManager::pushUndo(const QVector<QImage>& layers)
{
    // Push undo up to 50 undos
    undoLayerStack.push(layers);
    if (undoLayerStack.size() > 50) undoLayerStack.remove(0);

    undoSelectionStack.push(selectionOverlay);
    if (undoSelectionStack.size() > 50) undoSelectionStack.remove(0);

    undoSelectionPathStack.push(selectionsPath);
    if (undoSelectionPathStack.size() > 50) undoSelectionPathStack.remove(0);
}

void UndoManager::undo()
{
    // Check if stacks are large enough to undo
    if (undoLayerStack.size() <= 1) return;
    if (undoSelectionStack.size() <= 1) return;
    if (undoSelectionPathStack.size() <= 1) return;

    // update redo stack with new undo and push undo stack to top
    redoLayerStack.push(undoLayerStack.pop());
    layers = undoLayerStack.top();
    layerManager->layers = layers;

    redoSelectionStack.push(undoSelectionStack.pop());
    selectionOverlay = undoSelectionStack.top();
    layerManager->selectionOverlay = selectionOverlay;

    
    redoSelectionPathStack.push(undoSelectionPathStack.pop());
    selectionsPath = undoSelectionPathStack.top();
    layerManager->selectionsPath = selectionsPath;
    layerManager->undo();
    layerManager->update();
    
    //update Current layer selected on UI
    int currentRow = layerManager->layersList->currentRow();
    if (currentRow <= 0)
    {
        layerManager->layersList->setCurrentRow(0);
    }
    else if (currentRow >= layerManager->layers.count())
    {
        layerManager->layersList->setCurrentRow(currentRow - 1);
    }
    update();
}

// General Redo
void UndoManager::redo()
{
    // Check if Stacks are empty to see if redo is applicable
    if (redoLayerStack.isEmpty()) return;
    if (redoSelectionStack.isEmpty()) return;
    if (redoSelectionPathStack.isEmpty()) return;

    // Redo all shared Variables
    layers = redoLayerStack.pop();
    undoLayerStack.push(layers);
    layerManager->layers = layers;

    selectionOverlay = redoSelectionStack.pop();
    undoSelectionStack.push(selectionOverlay);
    layerManager->selectionOverlay = selectionOverlay;

    selectionsPath = redoSelectionPathStack.pop();
    undoSelectionPathStack.push(selectionsPath);
    layerManager->selectionsPath = selectionsPath;

    layerManager->redo();
    layerManager->update();
    update();
}