#include <MenuBar.h>

MenuBar::MenuBar(QWidget* parent)
    : QWidget(parent)
{

    setWindowTitle("Manager");
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);

  
    QVBoxLayout* layout = new QVBoxLayout(this);

    QMenuBar* menuBar = new QMenuBar(this);
    layout->setMenuBar(menuBar);

    //QMenu* fileMenu = menuBar->addMenu("File");
    QMenu* editMenu = menuBar->addMenu("Edit");
    QMenu* selectionMenu = menuBar->addMenu("Selections");
    QMenu* brushMenu = menuBar->addMenu("Brushes");



    QAction* undoAction = new QAction("&Undo", this);
    connect(undoAction, &QAction::triggered, this, [&]() {
        emit undoPressed();
        });
    editMenu->addAction(undoAction);

    QAction* redoAction = new QAction("&Redo", this);
    connect(redoAction, &QAction::triggered, this, [&]() {
        emit redoPressed();
        });
    editMenu->addAction(redoAction);

    QAction* clearAction = new QAction("&Clear Layer or Selection", this);
    connect(clearAction, &QAction::triggered, this, [&]() {
        emit clearPressed();
        });
    editMenu->addAction(clearAction);



    //QAction* selectAllAction = new QAction("&Select All", this);
    //connect(selectAllAction, &QAction::triggered, this, [&]() {
    //    emit selectAllPressed();
    //    });
    //selectionMenu->addAction(selectAllAction);

    //QAction* clearSelectionAction = new QAction("&Clear Selections", this);
    //connect(clearSelectionAction, &QAction::triggered, this, [&]() {
    //    emit clearSelectionPressed();
    //    });
    //selectionMenu->addAction(clearSelectionAction);

    QAction* expandSelectionAction = new QAction("&Expand Selections", this);
    connect(expandSelectionAction, &QAction::triggered, this, [&]() {
        emit expandSelectionsPressed();
        });
    selectionMenu->addAction(expandSelectionAction);

    QAction* contractSelectionAction = new QAction("&Contract Selections", this);
    connect(contractSelectionAction, &QAction::triggered, this, [&]() {
        emit contractSelectionsPressed();
        });
    selectionMenu->addAction(contractSelectionAction);



    QAction* openBrushAction = new QAction("&Open Brush Menu", this);
    connect(openBrushAction, &QAction::triggered, this, [&]() {
        emit openBrushMenu();
        });
    brushMenu->addAction(openBrushAction);

}