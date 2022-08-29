#include "stdafx.h"

#include "Workspace.h"
#include "Shape.h"
#include "MainWindow.h"
#include "NodeSearcher.h"
#include <QPainter>
#include <functional>

Workspace::Workspace(QWidget* parent, const FormatType type, NodeSearcher* nodeSearcher)
	: QWidget(parent),
	type_(type),
	selectedNode_(nullptr),
	targetPos_(0.0, 0.0),
	offset_(0.0, 0.0),
	startPan_(0.0, 0.0),
	scale_(1.0),
	shapeFactory_(new ShapeFactory<Line>()),
	nodeSearcher_(nodeSearcher),
	bIsCtrlPressed_(false),
	bIsShiftPressed_(false),
	bIsAltPressed_(false),
	nodesOnLines_(nullptr, nullptr),
	currentState_(State::NONE)
{
	setFocusPolicy(Qt::StrongFocus);
	setAttribute(Qt::WA_StaticContents);
	setMouseTracking(true);

	InitializeStates_();
}

Workspace::~Workspace() { }

void Workspace::ResetTransform()
{
	offset_ = Vector2D();
	startPan_ = Vector2D();

	const auto* wsSettings = WorkspaceSettings::Instance();
	scale_ = currentSize_.x / wsSettings->GetMaxWorkspaceSize().x;

	update();
}

QString Workspace::GetTargetPositionAsString() const
{
	const auto* wsSettings = WorkspaceSettings::Instance();

	const auto centre = wsSettings->GetMaxWorkspaceSize() / 2.0;
	const auto difference = ScreenToWorld(targetPos_) - centre;

	QString result;

	if (difference.x <= 0.0 && difference.y >= 0.0)
		result = QString("X: %0\tY: %1");
	else if (difference.x <= 0.0 && difference.y <= 0.0)
		result = QString("X: %0\tZ: %1");
	else if (difference.x >= 0.0 && difference.y <= 0.0)
		result = QString("Y: %0\tZ: %1");


	if (!result.isEmpty())
	{
		const auto temp = difference.Abs() * wsSettings->GetFactor(type_);

		return result
		.arg(temp.x, 0, 'f', 1)
		.arg(temp.y, 0, 'f', 1);
	}

	return QString{};
}

QString Workspace::GetSelectedShapeInfoAsString() const
{
	if (selectedShape_ == nullptr)
		return QString{};

	const auto* wsSettings = WorkspaceSettings::Instance();
	const auto factor = wsSettings->GetFactor(type_);

	return selectedShape_->GetSizeAsString(factor);
}

constexpr void Workspace::InitializeStates_()
{
	constexpr auto indexState_NONE = static_cast<size_t>(State::NONE);
	states_[indexState_NONE].OnMouseRelease = &Workspace::ST_NONE_OnMouseRelease_;

	constexpr auto indexState_SHAPE_MODIFICATION = static_cast<size_t>(State::SHAPE_MODIFICATION);
	states_[indexState_SHAPE_MODIFICATION].OnMouseRelease = &Workspace::ST_SHAPE_MODIFICATION_OnMouseRelease_;
	states_[indexState_SHAPE_MODIFICATION].OnMouseMove = &Workspace::ST_SHAPE_MODIFICATION_OnMouseMove_;
}

void Workspace::ST_NONE_OnMouseRelease_(const QMouseEvent* event)
{
	switch (event->button())
	{
	case Qt::MouseButton::LeftButton:
	{
		const auto worldPos = ScreenToWorld(targetPos_);

		const auto mw = dynamic_cast<MainWindow*>(window());
		
		selectedShape_ = shapeFactory_->Create(mw->GetPen());

		selectedShape_->GetNextNode()->position = worldPos;
		selectedNode_ = selectedShape_->GetNextNode();
		selectedNode_->position = worldPos;

		currentState_ = State::SHAPE_MODIFICATION;

		break;
	}
	case Qt::MouseButton::RightButton:
	{
		const std::function fWorldToScreen = [this](const Vector2D& Location)
		{
			return WorldToScreen(Location);
		};

		for (auto shapeIt = shapes_.begin(); shapeIt != shapes_.end(); ++shapeIt)
		{
			auto* hitNode = shapeIt->get()->HasNode(targetPos_, fWorldToScreen);
			if (hitNode != nullptr)
			{
				std::unique_lock<std::mutex>(NodeSearchMutex);

				selectedShape_ = std::move(*shapeIt);
				selectedNode_ = hitNode;

				shapes_.erase(shapeIt);
				currentState_ = State::SHAPE_MODIFICATION;

				break;
			}
		}

		break;
	}
	default: break;
	}
}

void Workspace::ST_SHAPE_MODIFICATION_OnMouseRelease_(const QMouseEvent* event)
{
	switch (event->button())
	{
	case Qt::MouseButton::LeftButton:
	{
		auto* tempNode = selectedShape_->GetNextNode();
		if (tempNode != nullptr)
		{
			selectedNode_ = tempNode;
			return;
		}


		if (selectedNode_ != nullptr)
			shapes_.emplace_back(std::move(selectedShape_));

		selectedNode_ = nullptr;
		currentState_ = State::NONE;

		break;
	}
	default: break;
	}
}

void Workspace::ST_SHAPE_MODIFICATION_OnMouseMove_(const QMouseEvent* event)
{
	selectedNode_->position = ScreenToWorld(targetPos_);
	selectedNode_->parent->Update();
}

void Workspace::Serialize(QDataStream& out) const
{
	out << type_ << shapes_.size();

	for (const auto& shape : shapes_)
		shape->Serialize(out);
}

void Workspace::Deserialize(QDataStream& in)
{
	size_t shapeCount;
	in >> type_ >> shapeCount;

	shapes_.reserve(shapeCount);
	for (size_t i = 0; i < shapeCount; i++)
	{
		Shape::Type type;
		in >> type;

		std::unique_ptr<Shape> newShape;

		switch (type)
		{
		case Shape::Type::LINE: newShape = std::make_unique<Line>(); break;
		case Shape::Type::BOX: newShape = std::make_unique<Box>(); break;
		case Shape::Type::CIRCLE: newShape = std::make_unique<Circle>(); break;
		case Shape::Type::OVAL: newShape = std::make_unique<Oval>(); break;
		case Shape::Type::CURVE: newShape = std::make_unique<Curve>(); break;
		case Shape::Type::SECTOR: newShape = std::make_unique<Sector>(); break;
		}

		if (newShape != nullptr)
		{
			newShape->Deserialize(in);
			shapes_.emplace_back(std::move(newShape));
		}
	}

	update();
}

void Workspace::DrawFrame(QPainter* painter) const
{
	const auto* wsSettings = WorkspaceSettings::Instance();
	const auto [w, h] = wsSettings->GetFormatSizeByType(type_);

	const std::array rects = {
		QRectF(QPointF(20.0, 5.0), QPointF(w - 5.0, h - 5.0)),
		QRectF(QPointF(w - 150.0, h - 27.0), QPointF(w - 5.0, h - 5.0)) };

	const std::array lines = {
		QLineF(QPointF(w - 150.0, h - 20.0), QPointF(w - 80.0, h - 20.0)),
		QLineF(QPointF(w - 150.0, h - 13.0), QPointF(w - 5.0, h - 13.0)),
		QLineF(QPointF(w - 125.0, h - 27.0), QPointF(w - 125.0, h - 13.0)),
		QLineF(QPointF(w - 95.0, h - 27.0), QPointF(w - 95.0, h - 13.0)),
		QLineF(QPointF(w - 80.0, h - 27.0), QPointF(w - 80.0, h - 5.0)),
		QLineF(QPointF(w - 45.0, h - 13.0), QPointF(w - 45.0, h - 5.0)),
		QLineF(QPointF(w - 25.0, h - 13.0), QPointF(w - 25.0, h - 5.0)) };

	QPen framePen;
	framePen.setWidthF(0.5);
	painter->setPen(framePen);
	painter->drawRects(rects.data(), 2);
	painter->drawLines(lines.data(), 7);
}

void Workspace::paintEvent(QPaintEvent* event)
{
	QWidget::paintEvent(event);

	QPainter painter(this);

	painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
	painter.scale(scale_, scale_);
	painter.translate(offset_);

	DrawShapes(&painter);

	DrawHelpers_(&painter);
}

void Workspace::mousePressEvent(QMouseEvent* event)
{
	QWidget::mousePressEvent(event);

	startPan_ = event->position();
}

void Workspace::mouseReleaseEvent(QMouseEvent* event)
{
	QWidget::mouseReleaseEvent(event);

	const auto f = states_[static_cast<size_t>(currentState_)].OnMouseRelease;
	if (f != nullptr)
		(this->*f)(event);
}

void Workspace::mouseMoveEvent(QMouseEvent* event)
{
	QWidget::mouseMoveEvent(event);

	targetPos_ = event->position();

	const auto pressedButtons = event->buttons();
	if (pressedButtons.testFlag(Qt::MouseButton::MiddleButton))
	{
		offset_ += (event->position() - startPan_) / scale_;
		startPan_ = event->position();
	}

	if (nodeSearcher_ != nullptr && (bIsCtrlPressed_ || bIsShiftPressed_))
		nodeSearcher_->Run();

	UpdateSpecialKeys_();

	const auto f = states_[static_cast<size_t>(currentState_)].OnMouseMove;
	if (f != nullptr)
		(this->*f)(event);

	const auto* win = dynamic_cast<MainWindow*>(window());

	auto* coordinateLabel = win->GetCoordinateLabel();
	coordinateLabel->setText(GetTargetPositionAsString());

	auto* shapeInfoLabel = win->GetShapeInfoLabel();
	shapeInfoLabel->setText(GetSelectedShapeInfoAsString());

	update();
}

void Workspace::wheelEvent(QWheelEvent* event)
{
	QWidget::wheelEvent(event);

	const auto mouseWorldPositionBeforeZoom = ScreenToWorld(event->position());

	if (event->angleDelta().x() > 0 || event->angleDelta().y() > 0)
		scale_ *= 1.1;
	else
		scale_ /= 1.1;

	scale_ = std::fmin(scale_, 20.0);
	scale_ = std::fmax(scale_, 0.15);

	const auto mouseWorldPositionAfterZoom = ScreenToWorld(event->position());

	offset_ -= mouseWorldPositionBeforeZoom - mouseWorldPositionAfterZoom;

	update();
}

void Workspace::keyPressEvent(QKeyEvent* event)
{
	QWidget::keyPressEvent(event);

	switch (event->key())
	{
	case Qt::Key_Control: 
		bIsCtrlPressed_ = true;
		break;
	case Qt::Key_Shift: 
		bIsShiftPressed_ = true;
		break;
	case Qt::Key_Alt: 
		bIsAltPressed_ = true;
		break;
	case Qt::Key_Delete:
		if (selectedShape_ != nullptr)
		{
			selectedShape_.reset();
			selectedNode_ = nullptr;
			currentState_ = State::NONE;

			update();
		}

		break;
	default: break;
	}
}

void Workspace::keyReleaseEvent(QKeyEvent* event)
{
	QWidget::keyReleaseEvent(event);

	switch (event->key())
	{
	case Qt::Key_Control: 
		bIsCtrlPressed_ = false;
		break;
	case Qt::Key_Shift: 
		bIsShiftPressed_ = false;
		break;
	case Qt::Key_Alt:
		bIsAltPressed_ = false;
		break;
	default: break;
	}
}

void Workspace::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);

	currentSize_.x = static_cast<double>(event->size().width());
	currentSize_.y = static_cast<double>(event->size().height());

	auto* wsSettings = WorkspaceSettings::Instance();
	const auto [w, h] = wsSettings->GetMaxWorkspaceSize();
	if (currentSize_.x >= w && currentSize_.y >= h)
	{
		wsSettings->SetMaxWorkspaceSize(currentSize_);
		return;
	}

	scale_ = currentSize_.x / w;
}

void Workspace::DrawHelpers_(QPainter* painter) const
{
	DrawHelperLines_(painter);
	DrawAxes_(painter);
	DrawNodes_(painter);
}

void Workspace::DrawShapes(QPainter* painter) const
{
	for (const auto& shape : shapes_)
		shape->Draw(painter);

	DrawHelperLines_(painter);

	if (selectedShape_ == nullptr)
		return;

	selectedShape_->Draw(painter);
	selectedShape_->DrawHelpers(painter);
}

void Workspace::DrawNodes_(QPainter* painter) const
{
	painter->setPen(Qt::NoPen);
	painter->setBrush(Qt::red);

	painter->drawEllipse(ScreenToWorld(targetPos_), 5.0, 5.0);
	painter->setBrush(Qt::NoBrush);
}

void Workspace::DrawHelperLines_(QPainter* painter) const
{
	const QPen pen(Qt::black, 1.0, Qt::DashLine);
	painter->setPen(pen);

	const auto [xNode, yNode] = nodesOnLines_;
	const auto worldTargetPosition = ScreenToWorld(targetPos_);
	if (xNode != nullptr)
	{
		const QLineF xLine(worldTargetPosition, xNode->position);
		painter->drawLines(&xLine, 1);
	}

	if (yNode != nullptr)
	{
		const QLineF yLine(worldTargetPosition, yNode->position);
		painter->drawLines(&yLine, 1);
	}
}

void Workspace::DrawAxes_(QPainter* painter) const
{
	const auto bottomRight = ScreenToWorld(currentSize_);
	const auto topLeft = ScreenToWorld(Vector2D());

	const auto* wsSettings = WorkspaceSettings::Instance();
	const auto centre = wsSettings->GetMaxWorkspaceSize() / 2.0;


	const QLineF lines[2] = {
		QLineF(QPointF(centre.x, topLeft.y), QPointF(centre.x, bottomRight.y)),
		QLineF(QPointF(topLeft.x, centre.y), QPointF(bottomRight.x, centre.y)) };

	painter->setPen(QPen());
	painter->drawLines(lines, 2);
}

void Workspace::UpdateSpecialKeys_()
{
	const auto [nearestNode, fromX, fromY] = nodeSearcher_->GetLatestSearchResult();
	nodesOnLines_ = std::make_pair(nullptr, nullptr);

	if (bIsAltPressed_)
		UpdateStraightLine_();
	else if (bIsShiftPressed_)
		UpdateNodeOnLines_(fromX, fromY);
	else if (bIsCtrlPressed_)
		UpdateNearestNode_(nearestNode);
}

void Workspace::UpdateNodeOnLines_(Node* fromX, Node* fromY)
{
	if (fromX != nullptr)
	{
		const auto NodeXScreenPosition = WorldToScreen(fromX->position);
		const auto distanceToX = (targetPos_ - NodeXScreenPosition).Abs().x;
		if (distanceToX < 20.0)
		{
			targetPos_.x = NodeXScreenPosition.x;
			nodesOnLines_.first = fromX;
		}

	}

	if (fromY != nullptr)
	{
		const auto NodeYScreenPosition = WorldToScreen(fromY->position);
		const auto distanceToY = (targetPos_ - NodeYScreenPosition).Abs().y;
		if (distanceToY < 20.0)
		{
			targetPos_.y = NodeYScreenPosition.y;
			nodesOnLines_.second = fromY;
		}
	}
}

void Workspace::UpdateNearestNode_(const Node* nearestNode)
{
	if (nearestNode == nullptr)
		return;

	const auto distanceToNode = Vector2D::Distance(targetPos_, WorldToScreen(nearestNode->position));
	if (distanceToNode < 20.0)
		targetPos_ = WorldToScreen(nearestNode->position);
}

void Workspace::UpdateStraightLine_()
{
	if (selectedShape_ == nullptr)
		return;

	const auto previousNode = selectedShape_->GetOrientationNode(selectedNode_);
	if (previousNode == nullptr)
		return;

	const auto previousNodeLocationScreen = WorldToScreen(previousNode->position);
	const auto vectorToPreviousNode = (targetPos_ - previousNodeLocationScreen).Abs();

	if (vectorToPreviousNode.x > vectorToPreviousNode.y)
		targetPos_.y = previousNodeLocationScreen.y;
	else
		targetPos_.x = previousNodeLocationScreen.x;
}
