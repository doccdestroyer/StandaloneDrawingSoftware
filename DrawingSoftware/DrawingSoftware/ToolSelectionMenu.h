#pragma once

#include <QMainWindow>
#include <QToolBar>
#include <QWidget>
#include <QIcon>
#include <QDir>
#include <QAction>
#include <QVBoxLayout>
#include <QDockWidget>
#include <QMenuBar>
#include <QActionGroup>

class ToolSelectionMenu : public QMainWindow
{
    Q_OBJECT
public:

    ToolSelectionMenu(QMainWindow* parent = nullptr);
    QDockWidget* dock;
    std::string selectedTool;

signals:
    void brushEnabled();
    void lassoEnabled();
    void bucketEnabled();
    void polygonalLassoEnabled();
    void magicWandEnabled();
    void rectangularSelectionEnabled();
    void ellipticalSelectionEnabled();
    void eraserEnabled();

    void brushDisabled();
    void lassoDisabled();
    void bucketDisabled();
    void polygonalLassoDisabled();
    void magicWandDisabled();
    void rectangularSelectionDisabled();
    void ellipticalSelectionDisabled();
    void eraserDisabled();

private:
    QIcon icon;

    QAction* brushAction;
    QAction* lassoAction;
    QAction* bucketAction;
    QAction* polygonalAction;
    QAction* rectangularSelectionAction;
    QAction* ellipticalSelectionAction;
    QAction* eraserAction;

    void createToolBar();

    void enableBrushTool();
    void enableLassoTool();
    void enableBucketTool();
    void enablePolygonalLassoTool();
    void enableMagicWandTool();
    void enableRectangularSelectionTool();
    void enableEllipticalSelectionTool();
    void enableEraserTool();

    void disableTool();
};