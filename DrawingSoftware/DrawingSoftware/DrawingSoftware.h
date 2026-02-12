#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_DrawingSoftware.h"

class DrawingSoftware : public QMainWindow
{
    Q_OBJECT

public:
    DrawingSoftware(QWidget *parent = nullptr);
    ~DrawingSoftware();

private:
    Ui::DrawingSoftwareClass ui;
};

