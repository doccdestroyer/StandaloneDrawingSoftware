#pragma once

#include <QDial>
#include <QPainter>
#include <QVector2D>
#include <QMouseEvent>
#include <QObject>


class HueDial : public QDial

{

    //signals:
    //    void colorDialAngle(float angle);
    Q_OBJECT
public:
    HueDial(QWidget* parent = nullptr);
    float getSaturation() const;
    float getLightness() const;

signals:
    void hueDialAngle(float angle);


private:
    QRectF outerRect;
    qreal thickness;
    QRectF innerRect;
    bool inRing;
    QPointF clickPosition;
    bool inTriangle;
    QPolygonF triangle;
    QPointF SLPosition;
    float maxDist;
    QPointF p1;
    QPointF p2;
    QPointF p3;
    QPointF offset;

    QColor currentColour;

protected:

    QPointF projectPointToSegment(
        const QPointF& p,
        const QPointF& a,
        const QPointF& b);

    QPointF clampToTriangle(const QPointF& mousePos);

    static void barycentricPoints(
        const QPointF& p1,
        const QPointF& p2,
        const QPointF& p3,
        const QPointF& p,
        float& w1, float& w2, float& w3);

    float angleFromMouse(const QPoint& pos);

    void mousePressEvent(QMouseEvent* event);

    void mouseMoveEvent(QMouseEvent* event);

    void mouseReleaseEvent(QMouseEvent* event) override;

    void paintEvent(QPaintEvent* event) override;
};

