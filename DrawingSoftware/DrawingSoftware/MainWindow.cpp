
#include <MainWindow.h>

MainWindow::MainWindow()
{
    setWindowTitle("Bad Apple");
    setFixedSize(1920, 1080);
    setStyleSheet("background-color: rgb(30,30,30);");
    setAttribute(Qt::WA_TabletTracking);
    setAttribute(Qt::WA_MouseTracking);
    setMouseTracking(true);

    //setFocusPolicy(Qt::StrongFocus);


    //pngBackground = QImage(QDir::currentPath() + "/Images/PNGBackground.png");

    //background = QImage(1100, 1100, QImage::Format_ARGB32_Premultiplied);
    //image = background;
    //image.fill(Qt::transparent);
    //originalImage = image;
    //background.fill(Qt::white);
    //layers = { background, image };

    //undoStack.push(layers);
    //brush = QImage(QDir::currentPath() + "/Images/ChalkRot.png");

    //brushOutline = QImage(QDir::currentPath() + "/Images/ChalkRot_Outline.png");
    uiManager = new UIManager(this);

    brushTool = new BrushTool(uiManager, this);
    lassoTool = new LassoTool(uiManager, this);
    bucketTool = new BucketTool(uiManager, this);
    polygonalLassoTool = new PolygonalLassoTool(uiManager, this);
    rectangularSelectionTool = new RectangularSelectionTool(uiManager, this);
    ellipticalSelectionTool = new EllipticalSelectionTool(uiManager, this);

    toolSelectionMenu = new ToolSelectionMenu(this);

    menuBar = uiManager->menuBar;



    colourWindow = uiManager->colourWindow;
    //colourWindow = brushTool->colourWindow;
    //colourWindow->show();

    layerManager = brushTool->layerManager;
    //layerManager->show();

    //toolSelectionMenu->show();
    createDockWindows();

    brushControlsWindow = new BrushControlsWindow(this);

    connect(menuBar, &MenuBar::undoPressed,
        this, [&]()
        {
            toolSpecificUndo();
        });
    connect(menuBar, &MenuBar::redoPressed,
        this, [&]()
        {
            toolSpecificRedo();
        });

    connect(menuBar, &MenuBar::openBrushMenu,
        this, [&]()
        {
            brushControlsWindow->show();
        });

    connect(menuBar, &MenuBar::clearSelectionPressed,
        this, [&]()
        {
            clearOverlay();
            layerManager->selectionOverlay.fill(Qt::transparent);
            layerManager->selectionsPath.clear();
            updateTool();
        });

    connect(menuBar, &MenuBar::contractSelectionsPressed,
        this, [&]()
        {
            contractSelections();
        });

    connect(menuBar, &MenuBar::expandSelectionsPressed,
        this, [&]()
        {
            expandSelections();
        });

    //connect(menuBar, &MenuBar::selectAllPressed,
    //    this, [&]()
    //    {
    //        clearOverlay();
    //        layerManager->selectionOverlay.fill(Qt::transparent);
    //        layerManager->selectionsPath.clear();

    //        QVector<QPainterPath> newPath = {QPainterPath()};
    //        newPath[0].addRect(layerManager->image.rect());
    //        layerManager->selectionsPath = newPath;
    //        updateOverlay();
    //        updateTool();

    //    });


    connect(brushControlsWindow, &BrushControlsWindow::chalkEnabled,
        this, [&]()
        {
            brushTool->brush = QImage(QDir::currentPath() + "/Images/ChalkRot.png");
            brushTool->brushOutline = QImage(QDir::currentPath() + "/Images/ChalkRot_Outline.png");
            brushTool->update();
        });
    connect(brushControlsWindow, &BrushControlsWindow::roundEnabled,
        this, [&]()
        {
            brushTool->brush = QImage(QDir::currentPath() + "/Images/CircleBrush.png");
            brushTool->brushOutline = QImage(QDir::currentPath() + "/Images/CircleBrush_Outline.png");
            brushTool->update();
        });
    connect(brushControlsWindow, &BrushControlsWindow::horizontalChalkEnabled,
        this, [&]()
        {
            brushTool->brush = QImage(QDir::currentPath() + "/Images/HorizontalBrush.png");
            brushTool->brushOutline = QImage(QDir::currentPath() + "/Images/HorizontalBrush_Outline.png");
            brushTool->update();
        });
    connect(brushControlsWindow, &BrushControlsWindow::airbrushEnabled,
        this, [&]()
        {
            brushTool->brush = QImage(QDir::currentPath() + "/Images/AirBrush.png");
            brushTool->brushOutline = QImage(QDir::currentPath() + "/Images/CircleBrush_Outline.png");
            brushTool->update();
        });
    connect(brushControlsWindow, &BrushControlsWindow::penEnabled,
        this, [&]()
        {
            brushTool->brush = QImage(QDir::currentPath() + "/Images/PenBrush.png");
            brushTool->brushOutline = QImage(QDir::currentPath() + "/Images/PenBrush_Outline.png");
            brushTool->update();
        });

    connect(brushControlsWindow, &BrushControlsWindow::scatteringEnabled,
        this, [&]()
        {
            brushTool->scatteringEnabled = true;
        });
    connect(brushControlsWindow, &BrushControlsWindow::scatteringDisabled,
        this, [&]()
        {
            brushTool->scatteringEnabled = false;
        });

    connect(brushControlsWindow, &BrushControlsWindow::tiltEnabled,
        this, [&]()
        {
            brushTool->tiltEnabled = true;
        });
    connect(brushControlsWindow, &BrushControlsWindow::tiltDisabled,
        this, [&]()
        {
            brushTool->tiltEnabled = false;
        });


    connect(brushControlsWindow, &BrushControlsWindow::penPressureSizeOn,
        this, [&]()
        {
            brushTool->pressureAffectsSize = true;
        });
    connect(brushControlsWindow, &BrushControlsWindow::penPressureOpacityOff,
        this, [&]()
        {
            brushTool->pressureAffectsSize = false;
        });

    connect(brushControlsWindow, &BrushControlsWindow::penPressureOpacityOn,
        this, [&]()
        {
            brushTool->pressureAffectsOpacity = true;
        });
    connect(brushControlsWindow, &BrushControlsWindow::penPressureOpacityOff,
        this, [&]()
        {
            brushTool->pressureAffectsOpacity = false;
        });

    connect(brushControlsWindow, &BrushControlsWindow::opacitySliderChanged,
        this, [=](int value)
        {
            brushTool->opacity = value/100.0;
            brushTool->update();
        });
    connect(brushControlsWindow, &BrushControlsWindow::sizeSliderChanged,
        this, [=](int value)
        {
            brushTool->brushSize = value;
            brushTool->update();
        });
    connect(brushControlsWindow, &BrushControlsWindow::scatteringSliderChanged,
        this, [=](int value)
        {
            if (brushTool->scatteringEnabled)
            {
                brushTool->spacing = value;
                brushTool->update();
            }
            else
            {
                brushTool->spacing = 0;
            }
        });

    connect(brushTool, &BrushTool::brushDisabled,
        this, [&]()
        {
            disableBrushTool();
        });

    connect(lassoTool, &LassoTool::lassoDisabled,
        this, [&]()
        {
            disableLassoTool();
        });
    connect(bucketTool, &BucketTool::bucketDisabled,
        this, [&]()
        {
            disableBucketTool();
        });
    connect(polygonalLassoTool, &PolygonalLassoTool::polygonalLassoDisabled,
        this, [&]()
        {
            disablePolygonalLassoTool();
        });




    connect(brushTool, &BrushTool::lassoEnabled,
        this, [&]()
        {
            enableLassoTool();
        });
    connect(brushTool, &BrushTool::bucketEnabled,
        this, [&]()
        {
            enableBucketTool();
        });
    connect(lassoTool, &LassoTool::brushEnabled,
        this, [&]()
        {
            enableBrushTool();
        });



    connect(toolSelectionMenu, &ToolSelectionMenu::brushEnabled,
        this, [&]()
        {
            enableBrushTool();
        });
    connect(toolSelectionMenu, &ToolSelectionMenu::lassoEnabled,
        this, [&]()
        {
            enableLassoTool();
        });
    connect(toolSelectionMenu, &ToolSelectionMenu::bucketEnabled,
        this, [&]()
        {
            enableBucketTool();
        });
    connect(toolSelectionMenu, &ToolSelectionMenu::polygonalLassoEnabled,
        this, [&]()
        {
            enablePolygonalLassoTool();
        });

    connect(toolSelectionMenu, &ToolSelectionMenu::rectangularSelectionEnabled,
        this, [&]()
        {
            enableRectangularSelectionTool();
        });

    connect(toolSelectionMenu, &ToolSelectionMenu::ellipticalSelectionEnabled,
        this, [&]()
        {
            enableEllipticalSelectionTool();
        });
    connect(toolSelectionMenu, &ToolSelectionMenu::eraserEnabled,
        this, [&]()
        {
            enableEraser();
        });
    connect(toolSelectionMenu, &ToolSelectionMenu::brushDisabled,
        this, [&]()
        {
            disableBrushTool();
        });
    connect(toolSelectionMenu, &ToolSelectionMenu::lassoDisabled,
        this, [&]()
        {
            disableLassoTool();
        });
    connect(toolSelectionMenu, &ToolSelectionMenu::bucketDisabled,
        this, [&]()
        {
            disableBucketTool();
        });
    connect(toolSelectionMenu, &ToolSelectionMenu::polygonalLassoDisabled,
        this, [&]()
        {
            disablePolygonalLassoTool();
        });
    connect(toolSelectionMenu, &ToolSelectionMenu::rectangularSelectionDisabled,
        this, [&]()
        {
            disableRectangularSelectionTool();
        });
    connect(toolSelectionMenu, &ToolSelectionMenu::ellipticalSelectionDisabled,
        this, [&]()
        {
            disableEllipticalSelectionTool();
        });
    connect(toolSelectionMenu, &ToolSelectionMenu::eraserDisabled,
        this, [&]()
        {
            disableEraser();
        });
}



void MainWindow::disablePolygonalLassoTool()
{
    polygonalLassoTool->points.clear();
    polygonalLassoTool->isComplete = false;
    polygonalLassoTool->isDrawing = false;
    polygonalLassoTool->isFirstClick = true;
    polygonalLassoTool->updateSelectionOverlay();

    layerManager->updateLayers(polygonalLassoTool->layers,
        polygonalLassoTool->overlay,
        polygonalLassoTool->zoomPercentage,
        polygonalLassoTool->panOffset,
        polygonalLassoTool->selectionsPath);
}
void MainWindow::disableBrushTool()
{
    layerManager->updateLayers(brushTool->layers,
        brushTool->overlay,
        brushTool->zoomPercentage,
        brushTool->panOffset,
        brushTool->selectionsPath);
}


void MainWindow::disableBucketTool()
{
    layerManager->updateLayers(bucketTool->layers,
        bucketTool->overlay,
        bucketTool->zoomPercentage,
        bucketTool->panOffset,
        bucketTool->selectionsPath);
}

void MainWindow::disableLassoTool()
{
    layerManager->updateLayers(lassoTool->layers,
        lassoTool->overlay,
        lassoTool->zoomPercentage,
        lassoTool->panOffset,
        lassoTool->selectionsPath);
}

void MainWindow::disableRectangularSelectionTool()
{
    layerManager->updateLayers(rectangularSelectionTool->layers,
        rectangularSelectionTool->overlay,
        rectangularSelectionTool->zoomPercentage,
        rectangularSelectionTool->panOffset,
        rectangularSelectionTool->selectionsPath);
}

void MainWindow::disableEllipticalSelectionTool()
{
    layerManager->updateLayers(ellipticalSelectionTool->layers,
        ellipticalSelectionTool->overlay,
        ellipticalSelectionTool->zoomPercentage,
        ellipticalSelectionTool->panOffset,
        ellipticalSelectionTool->selectionsPath);
}

void MainWindow::disableEraser()
{
    //disableBrushTool();
    layerManager->updateLayers(brushTool->layers,
        brushTool->overlay,
        brushTool->zoomPercentage,
        brushTool->panOffset,
        brushTool->selectionsPath);
    brushTool->isErasing = false;
}




void MainWindow::enableLassoTool()
{
    dock->setWidget(lassoTool);
    lassoTool->layers = layerManager->layers;
    lassoTool->zoomPercentage = layerManager->zoomPercentage;
    lassoTool->panOffset = layerManager->panOffset;
    lassoTool->selectionsPath = layerManager->selectionsPath;
    lassoTool->overlay = layerManager->selectionOverlay;
}

void MainWindow::enableBrushTool()
{
    dock->setWidget(brushTool);
    brushTool->layers = layerManager->layers;
    brushTool->zoomPercentage = layerManager->zoomPercentage;
    brushTool->panOffset = layerManager->panOffset;
    brushTool->selectionsPath = layerManager->selectionsPath;
    brushTool->overlay = layerManager->selectionOverlay;
    brushTool->isErasing = false;
}



void MainWindow::enablePolygonalLassoTool()
{
    dock->setWidget(polygonalLassoTool);
    polygonalLassoTool->layers = layerManager->layers;
    polygonalLassoTool->zoomPercentage = layerManager->zoomPercentage;
    polygonalLassoTool->panOffset = layerManager->panOffset;
    polygonalLassoTool->selectionsPath = layerManager->selectionsPath;
    polygonalLassoTool->overlay = layerManager->selectionOverlay;

    //polygonalLassoTool->updateSelectionOverlay();
}


void MainWindow::enableBucketTool()
{
    dock->setWidget(bucketTool);
    bucketTool->layers = layerManager->layers;
    bucketTool->zoomPercentage = layerManager->zoomPercentage;
    bucketTool->panOffset = layerManager->panOffset;
    bucketTool->selectionsPath = layerManager->selectionsPath;
    bucketTool->overlay = layerManager->selectionOverlay;
}

void MainWindow::enableRectangularSelectionTool()
{
    dock->setWidget(rectangularSelectionTool);
    rectangularSelectionTool->layers = layerManager->layers;
    rectangularSelectionTool->zoomPercentage = layerManager->zoomPercentage;
    rectangularSelectionTool->panOffset = layerManager->panOffset;
    rectangularSelectionTool->selectionsPath = layerManager->selectionsPath;
    rectangularSelectionTool->overlay = layerManager->selectionOverlay;
}

void MainWindow::enableEllipticalSelectionTool()
{
    dock->setWidget(ellipticalSelectionTool);
    ellipticalSelectionTool->layers = layerManager->layers;
    ellipticalSelectionTool->zoomPercentage = layerManager->zoomPercentage;
    ellipticalSelectionTool->panOffset = layerManager->panOffset;
    ellipticalSelectionTool->selectionsPath = layerManager->selectionsPath;
    ellipticalSelectionTool->overlay = layerManager->selectionOverlay;
}

void MainWindow::enableEraser()
{
    enableBrushTool();
    brushTool->isErasing = true;
}
void MainWindow::keyPressEvent(QKeyEvent* event)
{
    //if (event->key() == 61 && event->modifiers() & Qt::ControlModifier)
    //    brushTool->zoomIn();
    //if (event->key() == 45 && event->modifiers() & Qt::ControlModifier)
    //    brushTool->zoomOut();

    //if (event->key() == Qt::Key_0 && event->modifiers() & Qt::ControlModifier)
    //    brushTool->resetZoom();

    //if (event->key() == Qt::Key_Z && event->modifiers() & Qt::ControlModifier)
    //    brushTool->undo();
    //layerManager->undo();
    //if (event->key() == Qt::Key_Y && event->modifiers() & Qt::ControlModifier)
    //    brushTool->redo();
    //layerManager->redo();

    //if (event->key() == Qt::Key_Space)
    //{
    //    //panningEnabled = true;
    //    setCursor(Qt::OpenHandCursor);
    //}

    //if (event->key() == Qt::Key_E)
    //{
    //    /*  if (isErasing == true) {
    //          isErasing = false;
    //          colour = colourWindow->updateColour();
    //      }
    //      else {

    //          isErasing = true;
    //      }*/
    //}
    //if (event->key() == Qt::Key_L)
    //{
    //    layerManager->updateLayers(brushTool->layers, brushTool -> overlay);
    //    dock->setWidget(lassoTool);
    //    lassoTool->layers = layerManager->layers;
    //    lassoTool->overlay = layerManager->selectionOverlay;


    //}
    //if (event->key() == Qt::Key_B)
    //{
    //    layerManager->updateLayers(lassoTool->layers, lassoTool->overlay);
    //    dock->setWidget(brushTool);
    //    brushTool->layers = layerManager->layers;
    //    brushTool->overlay = layerManager->selectionOverlay;

    //    //if (brushType == "chalk") {

    //    //    brush = QImage(QDir::currentPath() + "/Images/CircleBrush.png");
    //    //    brushOutline = QImage(QDir::currentPath() + "/Images/CircleBrush_Outline.png");
    //    //    brushType = "circle";
    //    //}
    //    //else {
    //    //    brush = QImage(QDir::currentPath() + "/Images/ChalkRot.png");
    //    //    brushTool->brushOutline = QImage(QDir::currentPath() + "/Images/ChalkRot_Outline.png");
    //    //    brushType = "chalk";

    //    //}
    //}

    //if (event->key() == 91)
    //{
    //    //alterBrushSize(-1);
    //}

    //if (event->key() == 93)
    //{
    //    //alterBrushSize(1);
    //}

}

void MainWindow::createDockWindows()
{
    TopDock = new QDockWidget(this);
    TopDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea);
    TopDock->setFixedSize(1920, 50);

    dock = new QDockWidget(tr("Colour Window"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
   


    TopDock->setWidget(menuBar);
    addDockWidget(Qt::TopDockWidgetArea, TopDock);


    splitDockWidget(TopDock, dock, Qt::Vertical);


    //dock = new QDockWidget(tr("Colour Window"), this);
    colourWindow->setFixedSize(300, 300);
    dock->setWidget(colourWindow);
    addDockWidget(Qt::RightDockWidgetArea, dock);



    dock = new QDockWidget(tr("Layers"), this);
    dock->setWidget(layerManager);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    //QWidget* centralWidget = new QWidget();
    //QHBoxLayout* layout = new QHBoxLayout(centralWidget);

    QDockWidget* ToolDock = new QDockWidget(tr("Tools"), this);
    ToolDock->setWidget(toolSelectionMenu);
    addDockWidget(Qt::LeftDockWidgetArea, ToolDock);
    //layout->addWidget(toolSelectionMenu);

    dock = new QDockWidget(tr("WindowFileName"), this);
    dock->setWidget(brushTool);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
    //layout->addWidget(brushTool);
    //dock->setLayout(layout);

    splitDockWidget(ToolDock, dock, Qt::Horizontal);
    //addDockWidget(Qt::LeftDockWidgetArea, dock);

}

void MainWindow::alterSelectionsScale(float scaleFactor)
{
    QVector<QPainterPath> newSelectionPath = { QPainterPath() };

    for (int i = 0; i < layerManager->selectionsPath.length(); ++i)
    {
        QPainterPath& path = layerManager->selectionsPath[i];
        QVector<QPolygonF> allPolys = path.toFillPolygons();

        for (int k = 0; k < allPolys.count(); k++)
        {
            QPolygonF polyF = allPolys[k];

            QTransform transform = QTransform();
            QPointF center = polyF.boundingRect().center();
            transform.translate(center.x(), center.y());
            transform.scale(scaleFactor, scaleFactor);
            transform.translate(-center.x(), -center.y());

            QPolygonF newPolyF = transform.map(polyF);
            QPainterPath newPath = QPainterPath();
            newPath.addPolygon(newPolyF);
            newSelectionPath.append(newPath);
        }
    }
    layerManager->selectionsPath = newSelectionPath;
    updateTool();
    updateOverlay();
    updateTool();

}

void MainWindow::contractSelections()
{
    alterSelectionsScale(9.0 / 10.0); 

}
void MainWindow::expandSelections()
{
    alterSelectionsScale(10.0 / 9.0);
}

void MainWindow::toolSpecificUndo()
{
    if (toolSelectionMenu->selectedTool == "Brush")
    {
        brushTool->undo();
    }
    else if (toolSelectionMenu->selectedTool == "Lasso")
    {
        lassoTool->undo();
    }
    else if (toolSelectionMenu->selectedTool == "Bucket")
    {
        bucketTool->undo();
    }
    else if (toolSelectionMenu->selectedTool == "PolygonalLasso")
    {
        polygonalLassoTool->undo();
    }
    else if (toolSelectionMenu->selectedTool == "RectangularSelection")
    {
        rectangularSelectionTool->undo();
    }
    else if (toolSelectionMenu->selectedTool == "EllipticalSelection")
    {
        ellipticalSelectionTool->undo();
    }
    else if (toolSelectionMenu->selectedTool == "Eraser")
    {
        brushTool->undo();
    }
    updateTool();
    updateOverlay();
    updateTool();
    updateAllTools();
/*    updateTool();
    updateOverlay();
    updateTool()*/;
}

void MainWindow::toolSpecificRedo()
{
    if (toolSelectionMenu->selectedTool == "Brush")
    {
        brushTool->redo();
    }
    else if (toolSelectionMenu->selectedTool == "Lasso")
    {
        lassoTool->redo();
    }
    else if (toolSelectionMenu->selectedTool == "Bucket")
    {
        bucketTool->redo();
    }
    else if (toolSelectionMenu->selectedTool == "PolygonalLasso")
    {
        polygonalLassoTool->redo();
    }
    else if (toolSelectionMenu->selectedTool == "RectangularSelection")
    {
        rectangularSelectionTool->redo();
    }
    else if (toolSelectionMenu->selectedTool == "EllipticalSelection")
    {
        ellipticalSelectionTool->redo();
    }
    else if (toolSelectionMenu->selectedTool == "Eraser")
    {
        brushTool->redo();
    }
    updateOverlay();
    updateTool();
}

void MainWindow::updateTool()
{
    if (toolSelectionMenu->selectedTool == "Brush")
    {
        brushTool->selectionsPath = layerManager->selectionsPath;
        brushTool->update();
    }
    else if (toolSelectionMenu->selectedTool == "Lasso")
    {
        lassoTool->selectionsPath = layerManager->selectionsPath;
        lassoTool->update();
    }
    else if (toolSelectionMenu->selectedTool == "Bucket")
    {
        bucketTool->selectionsPath = layerManager->selectionsPath;
        bucketTool->update();
    }
    else if (toolSelectionMenu->selectedTool == "PolygonalLasso")
    {
        polygonalLassoTool->selectionsPath = layerManager->selectionsPath;
        polygonalLassoTool->update();
    }
    else if (toolSelectionMenu->selectedTool == "RectangularSelection")
    {
        rectangularSelectionTool->selectionsPath = layerManager->selectionsPath;
        rectangularSelectionTool->update();
    }
    else if (toolSelectionMenu->selectedTool == "EllipticalSelection")
    {
        ellipticalSelectionTool->selectionsPath = layerManager->selectionsPath;
        ellipticalSelectionTool->update();
    }
    else if (toolSelectionMenu->selectedTool == "Eraser")
    {
        brushTool->selectionsPath = layerManager->selectionsPath;
        brushTool->update();
    }
}

void MainWindow::updateAllTools()
{
    brushTool->selectionsPath = layerManager->selectionsPath;
    brushTool->update();
   
    lassoTool->selectionsPath = layerManager->selectionsPath;
    lassoTool->update();
   
    bucketTool->selectionsPath = layerManager->selectionsPath;
    bucketTool->update();
   
    polygonalLassoTool->selectionsPath = layerManager->selectionsPath;
    polygonalLassoTool->update();
   
    rectangularSelectionTool->selectionsPath = layerManager->selectionsPath;
    rectangularSelectionTool->update();
   
    ellipticalSelectionTool->selectionsPath = layerManager->selectionsPath;
    ellipticalSelectionTool->update();
   
    brushTool->selectionsPath = layerManager->selectionsPath;
    brushTool->update();
}

void MainWindow::clearOverlay()
{

    if (toolSelectionMenu->selectedTool == "Lasso")
    {
        lassoTool->selectionsPath.clear();
        lassoTool->clearSelectionOverlay();
    }
    else if (toolSelectionMenu->selectedTool == "PolygonalLasso")
    {
        polygonalLassoTool->selectionsPath.clear();
        polygonalLassoTool->clearSelectionOverlay();
    }
    else if (toolSelectionMenu->selectedTool == "RectangularSelection")
    {
        rectangularSelectionTool->selectionsPath.clear();
        rectangularSelectionTool->clearSelectionOverlay();
    }
    else if (toolSelectionMenu->selectedTool == "EllipticalSelection")
    {
        ellipticalSelectionTool->selectionsPath.clear();
        ellipticalSelectionTool->clearSelectionOverlay();
    }
}

void MainWindow::updateOverlay()
{

    if (toolSelectionMenu->selectedTool == "Lasso")
    {
        lassoTool->updateSelectionOverlay();
    }
    else if (toolSelectionMenu->selectedTool == "PolygonalLasso")
    {
        polygonalLassoTool->updateSelectionOverlay();
    }
    else if (toolSelectionMenu->selectedTool == "RectangularSelection")
    {
        rectangularSelectionTool->updateSelectionOverlay();
    }
    else if (toolSelectionMenu->selectedTool == "EllipticalSelection")
    {
        ellipticalSelectionTool->updateSelectionOverlay();
    }
}