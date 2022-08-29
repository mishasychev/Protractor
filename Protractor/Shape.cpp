#include "stdafx.h"

#include "Shape.h"

#include <QPainter>
#include <numbers>
#include <QPainterPath>
#include <ranges>

Shape::Shape(const Type type)
	: nodes_(0),
	currentNodeIndex_(0),
	type_(type)
{
}

Shape::Shape(const Type type, size_t maxNumberOfNodes, const QPen& pen)
	: nodes_(maxNumberOfNodes, Node(Vector2D(), this)),
	currentNodeIndex_(0),
	pen_(pen),
	type_(type)
{
}

Node* Shape::GetPreviousNode()
{
	if (currentNodeIndex_ < 2)
		return nullptr;

	return &nodes_[currentNodeIndex_ - 2];
}

Node* Shape::GetNextNode()
{
	if (nodes_.size() == currentNodeIndex_)
		return nullptr;

	return &nodes_[currentNodeIndex_++];
}

Node* Shape::HasNode(const Vector2D atScreen, const std::function<Vector2D(const Vector2D& v)>& WorldToScreen)
{
	const auto it = std::ranges::find_if(nodes_, [&](const Node& n) 
	{
		const auto distanceToNode = Vector2D::Distance(atScreen, WorldToScreen(n.position));
		return distanceToNode < 0.01;
	});
	return (it != nodes_.end()) ? &(*it) : nullptr;
}

void Shape::Serialize(QDataStream& out) const
{
	out << type_ << nodes_.size() << pen_;

	for (const auto& node : nodes_)
		out << node.position;
}

void Shape::Deserialize(QDataStream& in)
{
	in >> currentNodeIndex_ >> pen_;

	nodes_.assign(currentNodeIndex_, Node(Vector2D(), this));
	for (auto& node : nodes_)
		in >> node.position;

	Update();
}

Node* Line::GetOrientationNode(Node* selectedNode)
{
	return selectedNode == &nodes_[0] ? &nodes_[1] : &nodes_[0];
}

QString Line::GetSizeAsString(const qreal factor) const
{
	const auto length = Vector2D::Distance(nodes_.front().position, nodes_.back().position) * factor;

	return QString("Length: %0")
		.arg(length, 0, 'f', 1);
}

void Line::Update()
{
	line_ = QLineF(nodes_.front().position, nodes_.back().position);
}

void Line::Draw(QPainter* painter) const
{
	painter->setPen(pen_);
	painter->drawLines(&line_, 1);
}

QString Box::GetSizeAsString(const qreal factor) const
{
	const auto currentSize = Vector2D(rect_.width(), rect_.height()).Abs() * factor;

	return QString("Width: %0\tHeight: %1")
		.arg(currentSize.x, 0, 'f', 1)
		.arg(currentSize.y, 0, 'f', 1);
}

void Box::Update()
{
	rect_ = QRectF(nodes_.front().position, nodes_.back().position);
}

void Box::Draw(QPainter* painter) const
{
	painter->setPen(pen_);
	painter->drawRects(&rect_, 1);
}

Node* Circle::GetNextNode()
{
	if (nodes_.size() == currentNodeIndex_)
	{
		constexpr auto PI_2 = std::numbers::pi_v<double> / 2.0;
		constexpr auto PI_4 = std::numbers::pi_v<double> / 4.0;

		const auto& p1 = nodes_.front().position;
		auto& p2 = nodes_.back().position;

		const auto p2_1 = p2 - p1;
		auto angle = Vector2D::Angle(p2_1, Vector2D(1.0, 0.0));
		while (angle - PI_2 > 0.0)
			angle -= PI_2;

		auto rotateAngle = (angle < PI_4) ? angle : angle - PI_2;
		if (p2.y > p1.y)
			rotateAngle *= -1.0;

		p2 = p1 + p2_1.Rotate(rotateAngle);

		return nullptr;
	}

	return &nodes_[currentNodeIndex_++];
}

Node* Circle::GetOrientationNode(Node* selectedNode)
{
	return selectedNode == &nodes_[0] ? &nodes_[1] : &nodes_[0];
}

QString Circle::GetSizeAsString(const qreal factor) const
{
	const auto radius = Vector2D::Distance(nodes_.front().position, nodes_.back().position) * factor;

	return QString("Radius: %0")
		.arg(radius, 0, 'f', 1);
}

void Circle::Update()
{
	const auto& p1 = nodes_.front().position;
	const auto& p2 = nodes_.back().position;
	const auto radius = Vector2D::Distance(p1, p2);

	rect_ = QRectF(p1 - radius, p1 + radius);
	radius_ = QLineF(p1, p2);
}

void Circle::Draw(QPainter* painter) const
{
	painter->setPen(pen_);
	painter->drawEllipse(rect_);
}

void Circle::DrawHelpers(QPainter* painter) const
{
	painter->setPen(Qt::DashLine);
	painter->drawLines(&radius_, 1);
}

QString Oval::GetSizeAsString(const qreal factor) const
{
	const auto currentSize = Vector2D(rect_.width(), rect_.height()).Abs() / 2.0 * factor;

	return QString("RadiusX: %0\tRadiusY: %1")
		.arg(currentSize.x, 0, 'f', 1)
		.arg(currentSize.y, 0, 'f', 1);
}

void Oval::Update()
{
	rect_ = QRectF(nodes_.front().position, nodes_.back().position);
}

void Oval::Draw(QPainter* painter) const
{
	painter->setPen(pen_);
	painter->drawEllipse(rect_);
}

void Oval::DrawHelpers(QPainter* painter) const
{
	painter->setPen(Qt::DashLine);
	painter->drawRects(&rect_, 1);
}

Node* Curve::GetOrientationNode(Node* selectedNode)
{
	if (currentNodeIndex_ == 2)
		return selectedNode == &nodes_[0] ? &nodes_[1] : &nodes_[0];

	if (selectedNode != &nodes_[2])
		return &nodes_[2];

	return nullptr;
}

void Curve::Update()
{
	path_.clear();

	const auto& p1 = nodes_.front().position;
	const auto& p2 = nodes_[1].position;

	if (currentNodeIndex_ == 2)
	{
		path_.moveTo(p1);
		path_.lineTo(p2);
	}
	else
	{
		const auto& p3 = nodes_.back().position;

		path_.moveTo(p1);
		path_.cubicTo(p1, p3, p2);

		helpLine1_ = QLineF(p1, p3);
		helpLine2_ = QLineF(p2, p3);
	}
}

void Curve::Draw(QPainter* painter) const
{
	painter->setPen(pen_);
	painter->drawPath(path_);
}

void Curve::DrawHelpers(QPainter* painter) const
{
	painter->setPen(Qt::DashLine);

	painter->drawLines(&helpLine1_, 1);
	painter->drawLines(&helpLine2_, 1);
}

Node* Sector::GetNextNode()
{
	if (nodes_.size() == currentNodeIndex_)
	{
		const auto& p1 = nodes_.front().position;
		const auto& p2 = nodes_[1].position;
		auto& p3 = nodes_.back().position;

		const auto radius = Vector2D::Distance(p1, p2);
		auto p3_1 = p3 - p1;
		p3_1 *= radius / p3_1.Length();

		p3 = p1 + p3_1;

		return nullptr;
	}

	return &nodes_[currentNodeIndex_++];
}

QString Sector::GetSizeAsString(const qreal factor) const
{
	const auto radius = Vector2D::Distance(nodes_.front().position, nodes_[1].position) * factor;

	if (currentNodeIndex_ == 2)
		return QString("Radius: %0").arg(radius, 0, 'f', 1);

	const auto angle = static_cast<double>(std::abs(sectorAngle_)) / 16.0;
	return QString("Radius: %0\tSector Angle: %1")
		.arg(radius, 0, 'f', 1)
		.arg(angle, 0, 'f', 1);
}

void Sector::Update()
{
	if (currentNodeIndex_ != 3)
		return;

	const auto& p1 = nodes_.front().position;
	const auto& p2 = nodes_[1].position;
	const auto& p3 = nodes_.back().position;

	const auto radius = Vector2D::Distance(p1, p2);
	rect_ = QRectF(p1 - radius, p1 + radius);

	constexpr auto factor = 16.0 * (180.0 / std::numbers::pi_v<double>);

	const auto p2_1Vector = p2 - p1;
	const auto p3_1Vector = p3 - p1;

	const auto startAngleD = Vector2D::Angle(p2_1Vector, Vector2D(1.0, 0.0)) * factor;
	startAngle_ = static_cast<qint32>(std::round(startAngleD));
	if (p2.y > p1.y)
		startAngle_ *= -1;

	const auto sectorAngleD = Vector2D::Angle(p3_1Vector, p2_1Vector);
	sectorAngle_ = static_cast<qint32>(sectorAngleD * factor);

	const auto rotatedVector = p2_1Vector.Rotate(sectorAngleD);

	if (Vector2D::Distance(rotatedVector, p3_1Vector * (radius / p3_1Vector.Length())) < 0.01)
		sectorAngle_ *= -1;
}

void Sector::Draw(QPainter* painter) const
{
	painter->setPen(pen_);

	if (currentNodeIndex_ == 2)
		painter->drawLine(nodes_.front().position, nodes_[1].position);
	else
		painter->drawPie(rect_, startAngle_, sectorAngle_);
}
