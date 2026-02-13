#pragma once

#include "HueDial.h"


HueDial::HueDial(QWidget* parent) : QDial(parent)
{

    {
        setMinimumSize(300, 300);
        setMaximumSize(300, 300);
        setWrapping(true);
        setStyleSheet("border: 0");

        // Repaint UI when dial moves
        connect(this, &QDial::valueChanged, this, [this](int change) {
            emit change;
            update();
            });

        QRectF windowArea = rect();
        qreal windowSide = qMin(windowArea.width(), windowArea.height());

        outerRect = QRect((width() - windowSide) / 2, (height() - windowSide) / 2, windowSide, windowSide);
        thickness = windowSide * 0.075;
        innerRect = outerRect.adjusted(thickness, thickness, -thickness, -thickness);
    }
}

float HueDial::getSaturation() const
{
    float w1, w2, w3;
    barycentricPoints(p1, p2, p3, SLPosition, w1, w2, w3);
    return qBound(0.0f, w2, 1.0f);

}
float HueDial::getLightness() const
{
    float w1, w2, w3;
    barycentricPoints(p1, p2, p3, SLPosition, w1, w2, w3);
    float lightness = w3 + 0.5f * w2;
    return qBound(0.0f, lightness, 1.0f);
}

// Lock dial location within triangle's edge even when dragged out of it
QPointF HueDial::projectPointToSegment(
    const QPointF& p,
    const QPointF& a,
    const QPointF& b)
{
    QVector2D ap(p - a);
    QVector2D ab(b - a);

    float abLenSq = QVector2D::dotProduct(ab, ab);
    if (abLenSq == 0.0f)
        return a;

    float t = QVector2D::dotProduct(ap, ab) / abLenSq;
    t = std::clamp(t, 0.0f, 1.0f);

    return a + (b - a) * t;
}

// General clamp to triangle when dragged outside of it
QPointF HueDial::clampToTriangle(const QPointF& mousePos)
{
    if (triangle.containsPoint(mousePos, Qt::OddEvenFill))
        return mousePos;

    QPointF pAB = projectPointToSegment(mousePos, p1, p2);
    QPointF pBC = projectPointToSegment(mousePos, p2, p3);
    QPointF pCA = projectPointToSegment(mousePos, p3, p1);

    float dAB = QLineF(mousePos, pAB).length();
    float dBC = QLineF(mousePos, pBC).length();
    float dCA = QLineF(mousePos, pCA).length();
    if (dAB <= dBC && dAB <= dCA) return pAB;
    if (dBC <= dCA)              return pBC;
    return pCA;
}


// Barycentric algebra used for triangular gradients/locations
void HueDial::barycentricPoints(
    const QPointF& p1,
    const QPointF& p2,
    const QPointF& p3,
    const QPointF& p,
    float& w1, float& w2, float& w3)
{
    float det =
        (p1.x() - p3.x()) * (p2.y() - p3.y()) -
        (p2.x() - p3.x()) * (p1.y() - p3.y());

    w1 =
        (p2.y() - p3.y()) * (p.x() - p3.x()) +
        (p3.x() - p2.x()) * (p.y() - p3.y());

    w2 =
        (p3.y() - p1.y()) * (p.x() - p3.x()) +
        (p1.x() - p3.x()) * (p.y() - p3.y());

    w1 /= det;
    w2 /= det;
    w3 = 1.0f - w1 - w2;
}

// Get angle on the dial based on mouse click position
float HueDial::angleFromMouse(const QPoint& pos)
{
    QPointF center = rect().center();
    QPointF d = pos - center;

    float angle = std::atan2(d.y(), d.x());
    angle = qRadiansToDegrees(angle);

    if (angle < 0)
        angle += 360.0f;

    emit angle;
    return angle;
}

void HueDial::mousePressEvent(QMouseEvent* event)
{
    triangle.clear();
    qreal windowSide = qMin(width(), height());
    qreal outerRadius = windowSide / 2.0;
    qreal thickness = windowSide * 0.075;
    qreal innerRadius = outerRadius - thickness;

    clickPosition = QPointF(event->pos());

    // Establish "inRing" boundaries by locaiton
    QPointF center = rect().center();
    QPointF delta = clickPosition - center;
    qreal dist = std::hypot(delta.x(), delta.y());
    inRing = (dist >= innerRadius) && (dist <= outerRadius);

    // Calculate ring dimensions used for triangle calculation
    float r = qMin(width(), height()) * 0.735f;
    float h = std::sqrt(3.0f) * r / 2.0f;

    p1 = QPointF(-r / 2, h / 3);
    p2 = QPointF(r / 2, h / 3);
    p3 = QPointF(0, -2 * h / 3);

    // Rotate triangle and poinbts relative to the centre
    QTransform t;
    t.translate(center.x(), center.y());
    t.rotate(-22.5);
    t.translate(-center.x(), -center.y());

    p1 = t.map(p1 + center);
    p2 = t.map(p2 + center);
    p3 = t.map(p3 + center);

    // Create triangle
    triangle << p1 << p2 << p3;

    // Check if in hue ring or lightness/saturation triangle
    if (inRing) {
        setValue(angleFromMouse(event->pos()));
        inTriangle = false;
    }
    else {
        if (triangle.containsPoint(clickPosition, Qt::OddEvenFill)) {
            inTriangle = true;
        }
        else {
            inTriangle = false;

        }
    }
}

void HueDial::mouseMoveEvent(QMouseEvent* event)
{
    QPointF mousePos = event->pos();
    clickPosition = clampToTriangle(mousePos);
    if (inRing) {
        setValue(angleFromMouse(event->pos()));
    }

    else if (inTriangle) if ((triangle.containsPoint(clickPosition, Qt::OddEvenFill))) {
        SLPosition = clickPosition;
        offset = rect().center() - SLPosition;
    }

    update();

}
void HueDial::mouseReleaseEvent(QMouseEvent* event)
{
    inRing = false;
    Q_UNUSED(event);
    inTriangle = false;
}

void HueDial::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter p(this);
    QPainter painterHS(this);

    p.setRenderHint(QPainter::Antialiasing);

    QRectF windowArea = rect();
    qreal windowSide = qMin(windowArea.width(), windowArea.height());

    // Dial knob locations
    qreal knobRadius = windowSide * 0.028;
    qreal knobDistance = windowSide / 2.135 - windowSide * 0.008;

    QPointF center = rect().center();
    p.setPen(QPen(Qt::white, 2));
    p.setBrush(Qt::NoBrush);

    // Draw two dials for H/SL
    p.save();
    p.translate(center);
    p.rotate(value());
    QPointF knobCenter = QPointF(knobDistance, 1.0);
    p.drawEllipse(knobCenter, knobRadius, knobRadius);
    p.restore();

    p.save();
    p.drawEllipse(center - offset, knobRadius, knobRadius);
    p.restore();
}