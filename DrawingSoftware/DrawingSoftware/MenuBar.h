#pragma once

#include <QWidget>
#include <QMenuBar>
#include <QMenu>
#include <QVBoxLayout>

class MenuBar : public QWidget
{
    Q_OBJECT

public:
    explicit MenuBar(QWidget* parent = nullptr);

signals:
    void undoPressed();
    void redoPressed();
    void clearPressed();

    //void selectAllPressed();
    void clearSelectionPressed();
    void expandSelectionsPressed();
    void contractSelectionsPressed();

    void openBrushMenu();
};