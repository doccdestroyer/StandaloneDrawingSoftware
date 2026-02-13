#include <ToolSelectionMenu.h>

ToolSelectionMenu::ToolSelectionMenu(QMainWindow* parent) : QMainWindow(parent)
{
	setWindowFlags(Qt::WindowStaysOnTopHint);
	setFixedSize(80, 1000);
	setWindowTitle("Tools");

	selectedTool = "Brush";
	//refreshTool();

	createToolBar();
}

void ToolSelectionMenu::createToolBar()
{
	QToolBar* toolBar = addToolBar(tr("Tools"));

	toolBar->setMovable(false);
	toolBar->setFloatable(false);
	toolBar->setOrientation(Qt::Vertical);
	toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
	toolBar->setAllowedAreas(Qt::LeftToolBarArea | Qt::RightToolBarArea);

	brushAction = new QAction(
		QIcon(QDir::currentPath() + "/Images/Icons/pen.png"),
		tr("&BrushTool"),
		this);
	brushAction->setStatusTip(tr("&Brush Tool"));;
	brushAction->setCheckable(true);
	connect(brushAction, &QAction::triggered, this, [&]() {
		enableBrushTool();
		});
	toolBar->addAction(brushAction);


	lassoAction = new QAction(
		QIcon(QDir::currentPath() + "/Images/Icons/lasso.png"),
		tr("&LassoTool"),
		this);
	lassoAction->setStatusTip(tr("&Lasso Tool"));;
	lassoAction->setCheckable(true);
	connect(lassoAction, &QAction::triggered, this, [&]() {
		enableLassoTool();
		});
	toolBar->addAction(lassoAction);

	bucketAction = new QAction(
		QIcon(QDir::currentPath() + "/Images/Icons/bucket.png"),
		tr("&BucketTool"),
		this);
	bucketAction->setStatusTip(tr("&Bucket Tool"));;
	bucketAction->setCheckable(true);
	connect(bucketAction, &QAction::triggered, this, [&]() {
		enableBucketTool();
		});
	toolBar->addAction(bucketAction);

	polygonalAction = new QAction(
		QIcon(QDir::currentPath() + "/Images/Icons/polylasso.png"),
		tr("&PolygonalLasso"),
		this);
	polygonalAction->setStatusTip(tr("&Polygonal Lasso Tool"));
	polygonalAction->setCheckable(true);
	connect(polygonalAction, &QAction::triggered, this, [&]() {
		enablePolygonalLassoTool();
		});
	toolBar->addAction(polygonalAction);

	rectangularSelectionAction = new QAction(
		QIcon(QDir::currentPath() + "/Images/Icons/rectangle.png"),
		tr("&RetangularSelection"),
		this);
	rectangularSelectionAction->setStatusTip(tr("&Rectangular Selection Tool"));
	rectangularSelectionAction->setCheckable(true);
	connect(rectangularSelectionAction, &QAction::triggered, this, [&]() {
		enableRectangularSelectionTool();
		});
	toolBar->addAction(rectangularSelectionAction);

	ellipticalSelectionAction = new QAction(
		QIcon(QDir::currentPath() + "/Images/Icons/ellipse.png"),
		tr("&EllipticalSelection"),
		this);
	ellipticalSelectionAction->setStatusTip(tr("&Elliptical Selection Tool"));
	ellipticalSelectionAction->setCheckable(true);
	connect(ellipticalSelectionAction, &QAction::triggered, this, [&]() {
		enableEllipticalSelectionTool();
		});
	toolBar->addAction(ellipticalSelectionAction);

	eraserAction = new QAction(
		QIcon(QDir::currentPath() + "/Images/Icons/eraser.png"),
		tr("&Eraser"),
		this);
	eraserAction->setStatusTip(tr("&Eraser Tool"));
	eraserAction->setCheckable(true);
	connect(eraserAction, &QAction::triggered, this, [&]() {
		enableEraserTool();
		});
	toolBar->addAction(eraserAction);

	toolBar->setStyleSheet(R"(
    QToolButton:checked {
        background-color: #448aff;
        color: white;
        border: 2px solid #2962ff;
        border-radius: 4px;
    }
	)");
	brushAction->setChecked(true);
}

void ToolSelectionMenu::disableTool()
{
	if (selectedTool == "Brush")
	{
		brushAction->setChecked(false);
		emit brushDisabled();
	}
	else if (selectedTool == "Lasso")
	{
		lassoAction->setChecked(false);
		emit lassoDisabled();
	}
	else if (selectedTool == "Bucket")
	{
		bucketAction->setChecked(false);
		emit bucketDisabled();
	}
	else if (selectedTool == "PolygonalLasso")
	{
		polygonalAction->setChecked(false);
		emit polygonalLassoDisabled();
	} 
	else if (selectedTool == "RectangularSelection")
	{
		rectangularSelectionAction->setChecked(false);
		emit rectangularSelectionDisabled();
	}
	else if (selectedTool == "EllipticalSelection")
	{
		ellipticalSelectionAction->setChecked(false);
		emit ellipticalSelectionDisabled();
	}
	else if (selectedTool == "Eraser")
	{
		eraserAction->setChecked(false);
		emit eraserDisabled();
	}
}

void ToolSelectionMenu::enableBrushTool()
{
	disableTool();
	emit brushEnabled();
	selectedTool = "Brush";
}

void ToolSelectionMenu::enableLassoTool()
{
	disableTool();
	emit lassoEnabled();
	selectedTool = "Lasso";
}

void ToolSelectionMenu::enableBucketTool()
{
	disableTool();
	emit bucketEnabled();
	selectedTool = "Bucket";
}

void ToolSelectionMenu::enablePolygonalLassoTool()
{
	disableTool();
	emit polygonalLassoEnabled();
	selectedTool = "PolygonalLasso";
}

void ToolSelectionMenu::enableMagicWandTool()
{
	disableTool();
	emit magicWandEnabled();
	selectedTool = "MagicWand";
}

void ToolSelectionMenu::enableRectangularSelectionTool()
{
	disableTool();
	emit rectangularSelectionEnabled();
	selectedTool = "RectangularSelection";
}

void ToolSelectionMenu::enableEllipticalSelectionTool()
{
	disableTool();
	emit ellipticalSelectionEnabled();
	selectedTool = "EllipticalSelection";
}

void ToolSelectionMenu::enableEraserTool()
{
	disableTool();
	emit eraserEnabled();
	selectedTool = "Eraser";

}
