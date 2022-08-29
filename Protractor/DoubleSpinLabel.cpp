#include "stdafx.h"

#include "DoubleSpinLabel.h"

#include <QWheelEvent>
#include <cmath>

DoubleSpinLabel::DoubleSpinLabel(QWidget* parent, qreal wheelStep, qreal mouseStep, qreal minimum, qreal maximum)
 : QLabel(parent),
	value_(minimum),
	wheelStep_(wheelStep),
	mouseStep_(mouseStep),
	minValue_(minimum),
	maxValue_(maximum)
{
	setFocusPolicy(Qt::FocusPolicy::WheelFocus);

	setCursor(QCursor(Qt::SizeHorCursor));

	UpdateText_();
}

void DoubleSpinLabel::wheelEvent(QWheelEvent* event)
{
	QLabel::wheelEvent(event);

	if (event->angleDelta().x() > 0 || event->angleDelta().y() > 0)
		ChangeValue_(wheelStep_);
	else
		ChangeValue_(-wheelStep_);

	UpdateText_();
}

void DoubleSpinLabel::mouseMoveEvent(QMouseEvent* event)
{
	QLabel::mouseMoveEvent(event);

	const bool bIsLeftButtonMoved = event->buttons().testFlag(Qt::MouseButton::LeftButton);
	if (!bIsLeftButtonMoved)
		return;

	if (event->position().x() - startPan_.x > 0.001)
		ChangeValue_(mouseStep_);
	else
		ChangeValue_(-mouseStep_);

	startPan_ = event->position();

	UpdateText_();
}

void DoubleSpinLabel::mousePressEvent(QMouseEvent* event)
{
	QLabel::mousePressEvent(event);

	startPan_ = event->position();
}

void DoubleSpinLabel::UpdateText_()
{
	setText(prefix_ + QString("%0").arg(value_, 0, 'f', 2) + suffix_);
}

void DoubleSpinLabel::ChangeValue_(qreal value)
{
	value_ += value;

	value_ = std::fmin(value_, maxValue_);
	value_ = std::fmax(value_, minValue_);
}
