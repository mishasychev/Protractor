#include "stdafx.h"

#include "LinePattern.h"

#include <QPainter>

void LinePattern::paintEvent(QPaintEvent* event)
{
	QToolButton::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QPen pen;
    pen.setCapStyle(Qt::RoundCap);
    pen.setWidthF(4.0);
    pen.setDashPattern(pattern_);
    painter.setPen(pen);

	const QPointF bottomRight(rect().width(), rect().height());

    painter.drawLine(10.0, bottomRight.y() / 2.0, bottomRight.x() - 10.0, bottomRight.y() / 2.0);
}
