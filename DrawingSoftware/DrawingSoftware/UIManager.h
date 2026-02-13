#pragma once
#pragma once

#include <ColourWindow.h>
#include <UndoManager.h>
#include <MenuBar.h>

class UIManager : public QWidget
{
    Q_OBJECT

public:
    explicit UIManager(QWidget* parent = nullptr);
    ColourWindow* colourWindow;
    UndoManager* undoManager;
    MenuBar* menuBar;
};