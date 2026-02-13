#include<UIManager.h>

UIManager::UIManager(QWidget* parent)
    : QWidget(parent)
{
    colourWindow = new ColourWindow(this);
    undoManager = new UndoManager(this);
    menuBar = new MenuBar(this);
};