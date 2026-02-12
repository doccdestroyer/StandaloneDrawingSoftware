#pragma once

#include <QWidget>
#include <QPainter>
#include <HueDial.h>
#include <QColor>
#include <QPainterPath>
#include <QObject>


class ColourWindow : public QWidget
{
    Q_OBJECT
public:

    ColourWindow(QWidget* parent = nullptr);

    int dialAngle;

    int hueAngle();

    QColor updateColour();

    QColor currentColour;

    bool hueOnlyUpdated;

signals:
    void colourChanged(const QColor& newColour);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void createWheel();
    //void mousePressEvent(QMouseEvent* event) override;

    void paintEvent(QPaintEvent*) override;

private:
    HueDial* dial = new HueDial(this);
    QRectF windowArea;

    float saturation = 255.0f;
    float lightness = 255.0f;
};


