#include "stdafx.h"

#include "PrintViewer.h"

#include <QPainter>

#include "Workspace.h"
#include "WorkspaceSettings.h"

PrintViewer::PrintViewer(QWidget* parent)
	: QWidget(parent),
	ws_(nullptr),
	scale_(1.0),
	isFrameOn_(false)
{
}

void PrintViewer::Initialize()
{
	const auto* wsSettings = WorkspaceSettings::Instance();

	const auto newSize = wsSettings->GetFormatSizeByType(ws_->GetFormatType()) * 2.0;
	setMinimumSize(static_cast<qint32>(newSize.x), static_cast<qint32>(newSize.y));

	scale_ = wsSettings->GetFactor(ws_->GetFormatType()) * 2.0;
}

void PrintViewer::paintEvent(QPaintEvent* event)
{
	QWidget::paintEvent(event);

	QPainter painter(this);

	painter.fillRect(rect(), Qt::white);

	if (isFrameOn_)
	{
		painter.scale(2.0, 2.0);
		ws_->DrawFrame(&painter);
	}

	painter.resetTransform();
	painter.scale(scale_, scale_);
	painter.translate(offset_);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

	ws_->DrawShapes(&painter);
}

void PrintViewer::mousePressEvent(QMouseEvent* event)
{
	QWidget::mousePressEvent(event);

	startPan_ = event->position();
}

void PrintViewer::mouseMoveEvent(QMouseEvent* event)
{
	QWidget::mouseMoveEvent(event);

	const auto pressedButtons = event->buttons();
	if (pressedButtons.testFlag(Qt::MouseButton::MiddleButton))
	{
		offset_ += (Vector2D(event->position()) - startPan_) / scale_;
		startPan_ = event->position();
	}

	update();
}