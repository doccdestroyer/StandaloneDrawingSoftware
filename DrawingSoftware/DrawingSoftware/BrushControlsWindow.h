#pragma once

#include <QMainWindow>
#include <QListWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDir>
#include <QIcon>
#include <QDockWidget>
#include <QToolBar>
#include <QCheckBox>
#include <QSlider>
#include <QString>
#include <QLabel>
class BrushControlsWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit BrushControlsWindow(QMainWindow* parent = nullptr);

signals:
	void chalkEnabled();
	void horizontalChalkEnabled();
	void roundEnabled();
	void penEnabled();
	void airbrushEnabled();

	void penPressureSizeOn();
	void penPressureOpacityOn();
	void scatteringEnabled();
	void tiltEnabled();

	void penPressureSizeOff();
	void penPressureOpacityOff();
	void scatteringDisabled();
	void tiltDisabled();

	void sizeSliderChanged(int value);
	void opacitySliderChanged(int value);
	void scatteringSliderChanged(int value);
	
private:
	//QDockWidget* dock;
	QListWidget* controlsList = nullptr;
	void onSettingClicked(QListWidgetItem* item);

	bool TiltEnabled = true;
	bool SizePressureEnabled = true;
	bool OpacityEnabled = false;
	bool ScatteringEnabled = false;
	//void callFunction(QLabel* sizeLabel, int value);
};