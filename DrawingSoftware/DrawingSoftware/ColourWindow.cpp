    #include <ColourWindow.h>

ColourWindow::ColourWindow(QWidget* parent) : QWidget(parent)

{
    setWindowTitle("Colour Wheel");

    setMinimumSize(300, 300);
    setMaximumSize(300, 300);
    resize(300, 300);
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
    createWheel();


    connect(dial, &HueDial::hueDialAngle,
        this, [=](int angle)
        {
            dialAngle = angle;
            updateColour();
        });

}

int ColourWindow::hueAngle()
{
    dialAngle = dial->value();
    update();
    dial->update();
    currentColour = updateColour();
    emit colourChanged(currentColour);
    return dial->value();
}

QColor ColourWindow::updateColour()
{

    saturation = dial->getSaturation();
    lightness = dial->getLightness();
    
    QColor newColor = QColor::fromHsl(360 - dialAngle, int(saturation * 255), int(lightness * 255));
    currentColour = newColor;
    emit colourChanged(currentColour);
    return QColor::fromHsl(360 - dialAngle, int(saturation * 255), int(lightness * 255));
}

void ColourWindow::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    dial->setGeometry(rect());
}

void ColourWindow::createWheel()
{
    dial->setRange(0, 360);
    dial->setGeometry(0, 0, 300, 300);
    int angle = dial->value();
}

void ColourWindow::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    windowArea = rect();
    qreal windowSide = qMin(windowArea.width(), windowArea.height());

    QRectF outerRect((width() - windowSide) / 2, (height() - windowSide) / 2, windowSide, windowSide);

    qreal thickness = windowSide * 0.075;
    QRectF innerRect = outerRect.adjusted(thickness, thickness, -thickness, -thickness);

    QPainterPath ring;
    ring.addEllipse(outerRect);
    ring.addEllipse(innerRect);

    ring.setFillRule(Qt::OddEvenFill);

    QConicalGradient gradient(rect().center(), 0);
    gradient.setColorAt(0.0, Qt::red);
    gradient.setColorAt(0.17, Qt::yellow);
    gradient.setColorAt(0.33, Qt::green);
    gradient.setColorAt(0.5, Qt::cyan);
    gradient.setColorAt(0.67, Qt::blue);
    gradient.setColorAt(0.83, Qt::magenta);
    gradient.setColorAt(1.0, Qt::red);

    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);

    painter.drawPath(ring);

    QPointF center = rect().center();

    float r = qMin(width(), height()) * 0.735f;
    float h = std::sqrt(3.0f) * r / 2.0f;

    QPointF p1(-r / 2, h / 3);
    QPointF p2(r / 2, h / 3);
    QPointF p3(0, -2 * h / 3);

    QImage triImg(windowSide, windowSide, QImage::Format_ARGB32);
    triImg.fill(Qt::transparent);

    QPointF imgCenter(windowSide / 2.0, windowSide / 2.0);

    QPointF a = p1 + imgCenter;
    QPointF b = p2 + imgCenter;
    QPointF c = p3 + imgCenter;


    QColor ca = Qt::black;
    QColor cb = QColor::fromHsv(360 - hueAngle(), 255, 255);
    QColor cc = Qt::white;

    float denom = (b.y() - c.y()) * (a.x() - c.x()) + (c.x() - b.x()) * (a.y() - c.y());

    for (int y = 0; y < windowSide; ++y)
    {
        for (int x = 0; x < windowSide; ++x)
        {
            QPointF p(x, y);

            float w1 = ((b.y() - c.y()) * (p.x() - c.x()) + (c.x() - b.x()) * (p.y() - c.y())) / denom;
            float w2 = ((c.y() - a.y()) * (p.x() - c.x()) + (a.x() - c.x()) * (p.y() - c.y())) / denom;
            float w3 = 1.0f - w1 - w2;

            if (w1 >= 0 && w2 >= 0 && w3 >= 0)
            {
                QColor col(
                    ca.red() * w1 + cb.red() * w2 + cc.red() * w3,
                    ca.green() * w1 + cb.green() * w2 + cc.green() * w3,
                    ca.blue() * w1 + cb.blue() * w2 + cc.blue() * w3
                );

                triImg.setPixelColor(x, y, col);
            }
        }
    }
    center = rect().center();
    painter.setRenderHint(QPainter::Antialiasing);

    painter.save();
    painter.resetTransform();
    painter.translate(center);
    painter.rotate(-22.5);
    painter.drawImage(QPointF(-triImg.width() / 2.0, -triImg.height() / 2.0), triImg);
    painter.restore();
}