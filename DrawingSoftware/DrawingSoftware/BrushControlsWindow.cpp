#include <BrushControlsWindow.h>

BrushControlsWindow::BrushControlsWindow(QMainWindow* parent)
	: QMainWindow(parent)
{
	setWindowTitle("Brush Controls");
	resize(300, 800);
	setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);

	controlsList = new QListWidget(this);
	controlsList->addItems({ "Brush Tip Shape"});
	controlsList->setSortingEnabled(true);
	controlsList->setMaximumSize(110, 300);

	QDockWidget* generalDock = new QDockWidget(this);
	generalDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	generalDock->setWidget(controlsList);
	addDockWidget(Qt::LeftDockWidgetArea, generalDock);
	QToolBar* toolBar = addToolBar(tr("Tools"));

	toolBar->setMovable(false);
	toolBar->setFloatable(false);
	toolBar->setOrientation(Qt::Vertical);
	toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
	toolBar->setAllowedAreas(Qt::LeftToolBarArea | Qt::RightToolBarArea);


	QCheckBox* affectSizeBox = new QCheckBox("Pen Pressure: Affects Size", this);
	affectSizeBox->setChecked(true);
	connect(affectSizeBox, &QCheckBox::toggled, [this](bool checked) {
		if (SizePressureEnabled)
		{
			emit penPressureSizeOff();
			SizePressureEnabled = false;
		}
		else
		{
			emit penPressureSizeOn();
			SizePressureEnabled = true;
		}
		});
	QSlider* sizeSlider = new QSlider(Qt::Horizontal, this);
	QLabel* sizeLabel = new QLabel("Size: 50", this);
	sizeSlider->setMinimum(1);
	sizeSlider->setMaximum(500);
	sizeSlider->setValue(50);
	connect(sizeSlider, &QSlider::valueChanged, this, [this, sizeLabel](int value) {
		sizeLabel->setText(QString("Size: ") + QString::number(value));
		emit sizeSliderChanged(value);
		});

	QCheckBox* affectOpacityBox = new QCheckBox("Pen Pressure: Affects Opacity", this);
	connect(affectOpacityBox, &QCheckBox::toggled, [this](bool checked) {
		if (OpacityEnabled)
		{
			emit penPressureOpacityOff();
			OpacityEnabled = false;
		}
		else
		{
			emit penPressureOpacityOn();
			OpacityEnabled = true;
		}
		});
	QSlider* opacitySlider = new QSlider(Qt::Horizontal, this);
	QLabel* opacityLabel = new QLabel("Opacity: 100%", this);
	opacitySlider->setMinimum(1);
	opacitySlider->setMaximum(100);
	opacitySlider->setValue(100);
	connect(opacitySlider, &QSlider::valueChanged, this, [this, opacityLabel](int value) {
		opacityLabel->setText(QString("Opacity: ") + QString::number(value) + "%");
		emit opacitySliderChanged(value);
		});

	QCheckBox* enableTiltBox = new QCheckBox("Enable Tilt", this);
	connect(enableTiltBox, &QCheckBox::toggled, [this](bool checked) {
		if (TiltEnabled)
		{
			emit tiltDisabled();
			TiltEnabled = false;
		}
		else
		{
			emit tiltEnabled();
			TiltEnabled = true;
		}
		});

	QAction* chalkAction = new QAction(
		QIcon(QDir::currentPath() + "/Images/BrushIcons/ChalkIcon.png"),
		tr("&BrushTool"),
		this);
	chalkAction->setStatusTip(tr("&Brush Tool"));
	connect(chalkAction, &QAction::triggered, this, [&]() {
		emit chalkEnabled();
		});
	toolBar->addAction(chalkAction);

	QAction* roundAction = new QAction(
		QIcon(QDir::currentPath() + "/Images/BrushIcons/HardIcon.png"),
		tr("&BrushTool"),
		this);
	roundAction->setStatusTip(tr("&Brush Tool"));
	connect(roundAction, &QAction::triggered, this, [&]() {
		emit roundEnabled();
		});
	toolBar->addAction(roundAction);

	QAction* airBrushAction = new QAction(
		QIcon(QDir::currentPath() + "/Images/BrushIcons/AirbrushIcon.png"),
		tr("&BrushTool"),
		this);
	airBrushAction->setStatusTip(tr("&Brush Tool"));
	connect(airBrushAction, &QAction::triggered, this, [&]() {
		emit airbrushEnabled();
		});
	toolBar->addAction(airBrushAction);

	QAction* horizontalAction = new QAction(
		QIcon(QDir::currentPath() + "/Images/BrushIcons/HorizontalIcon.png"),
		tr("&BrushTool"),
		this);
	horizontalAction->setStatusTip(tr("&Brush Tool"));
	connect(horizontalAction, &QAction::triggered, this, [&]() {
		emit horizontalChalkEnabled();
		});
	toolBar->addAction(horizontalAction);

	QAction* penAction = new QAction(
		QIcon(QDir::currentPath() + "/Images/BrushIcons/PenIcon.png"),
		tr("&BrushTool"),
		this);
	penAction->setStatusTip(tr("&Brush Tool"));
	connect(penAction, &QAction::triggered, this, [&]() {
		emit penEnabled();
		});
	toolBar->addAction(penAction);

	QDockWidget* detailDock = new QDockWidget(this);
	detailDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
	detailDock->setWidget(toolBar);
	addDockWidget(Qt::LeftDockWidgetArea, detailDock);
	detailDock->setFloating(false);

	QDockWidget* sizePressureDockBox = new QDockWidget(this);
	sizePressureDockBox->setFeatures(QDockWidget::NoDockWidgetFeatures);
	sizePressureDockBox->setWidget(affectSizeBox);
	addDockWidget(Qt::LeftDockWidgetArea, sizePressureDockBox);
	sizePressureDockBox->setFloating(false);

	QDockWidget* sizePressureDockLabel = new QDockWidget(this);
	sizePressureDockLabel->setFeatures(QDockWidget::NoDockWidgetFeatures);
	sizePressureDockLabel->setWidget(sizeLabel);
	addDockWidget(Qt::LeftDockWidgetArea, sizePressureDockLabel);
	sizePressureDockLabel->setFloating(false);

	QDockWidget* sizePressureDockSlider = new QDockWidget(this);
	sizePressureDockSlider->setFeatures(QDockWidget::NoDockWidgetFeatures);
	sizePressureDockSlider->setWidget(sizeSlider);
	addDockWidget(Qt::LeftDockWidgetArea, sizePressureDockSlider);
	sizePressureDockSlider->setFloating(false);

	QDockWidget* opacityPressureDockBox = new QDockWidget(this);
	opacityPressureDockBox->setFeatures(QDockWidget::NoDockWidgetFeatures);
	opacityPressureDockBox->setWidget(affectOpacityBox);
	addDockWidget(Qt::LeftDockWidgetArea, opacityPressureDockBox);
	opacityPressureDockBox->setFloating(false);

	QDockWidget* opacityPressureDockSlider = new QDockWidget(this);
	opacityPressureDockSlider->setFeatures(QDockWidget::NoDockWidgetFeatures);
	opacityPressureDockSlider->setWidget(opacitySlider);
	addDockWidget(Qt::LeftDockWidgetArea, opacityPressureDockSlider);
	opacityPressureDockSlider->setFloating(false);

	QDockWidget* opacityPressureDockLabel = new QDockWidget(this);
	opacityPressureDockLabel->setFeatures(QDockWidget::NoDockWidgetFeatures);
	opacityPressureDockLabel->setWidget(opacityLabel);
	addDockWidget(Qt::LeftDockWidgetArea, opacityPressureDockLabel);
	opacityPressureDockLabel->setFloating(false);
	
	QDockWidget* enableTiltDock = new QDockWidget(this);
	enableTiltDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
	enableTiltDock->setWidget(enableTiltBox);
	addDockWidget(Qt::LeftDockWidgetArea, enableTiltDock);
	enableTiltDock->setFloating(false);

	splitDockWidget(sizePressureDockSlider, sizePressureDockLabel, Qt::Horizontal);
	splitDockWidget(sizePressureDockBox, sizePressureDockSlider, Qt::Vertical);
	splitDockWidget(opacityPressureDockSlider, opacityPressureDockLabel, Qt::Horizontal);
	splitDockWidget(opacityPressureDockBox, opacityPressureDockSlider, Qt::Vertical);
	splitDockWidget(opacityPressureDockSlider, sizePressureDockBox, Qt::Vertical);
	splitDockWidget(sizePressureDockLabel, opacityPressureDockBox, Qt::Vertical);
	splitDockWidget(opacityPressureDockLabel, enableTiltDock, Qt::Vertical);
	splitDockWidget(detailDock, sizePressureDockBox, Qt::Horizontal);
	splitDockWidget(generalDock, detailDock, Qt::Horizontal);
}

void BrushControlsWindow::onSettingClicked(QListWidgetItem* item)
{
	if (!item) return;
	int index = controlsList->row(item);
}