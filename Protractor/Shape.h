#pragma once

#include <vector>
#include <functional>

class Shape;
class QPainter;
class Workspace;

class Node
{
public:
	constexpr Node() : parent(nullptr) {}
	constexpr Node(const Vector2D& pos, Shape* par) : position(pos), parent(par) {}
	constexpr Node(Vector2D&& pos, Shape* par) : position(std::move(pos)), parent(par) {}

	Vector2D position;
	Shape* parent;
};


class Shape
{
public:
	enum class Type : quint8
	{
		LINE,
		BOX,
		CIRCLE,
		OVAL,
		CURVE,
		SECTOR
	};

	Shape(Type type);

	Shape(const Type type, size_t maxNumberOfNodes, const QPen& pen);

	virtual void Update() {}

	virtual void Draw(QPainter* painter) const {}
	virtual void DrawHelpers(QPainter* painter) const {}

	auto& GetNodes() { return nodes_; }

	Node* GetPreviousNode();
	virtual Node* GetOrientationNode(Node* selectedNode) { return nullptr; };
	virtual  Node* GetNextNode();

	Node* HasNode(const Vector2D atScreen, const std::function<Vector2D(const Vector2D& v)>& WorldToScreen);

	virtual QString GetSizeAsString(const qreal factor) const { return QString(); }

	void Serialize(QDataStream& out) const;
	void Deserialize(QDataStream& in);

protected:
	std::vector<Node> nodes_;
	size_t currentNodeIndex_;

	QPen pen_;

private:
	Type type_;

public:
	Type GetType() const { return type_; }
};

class Line : public Shape
{
public:
	Line() : Shape(Shape::Type::LINE) {}

	Line(const QPen& pen) : Shape(Type::LINE, 2, pen) {}

	Node* GetOrientationNode(Node* selectedNode) override;

	QString GetSizeAsString(const qreal factor) const override;

	void Update() override;

	void Draw(QPainter* painter) const override;

private:
	QLineF line_;
};

class Box : public Shape
{
public:
	Box() : Shape(Shape::Type::BOX) {}

	Box(const QPen& pen) : Shape(Type::BOX, 2, pen) {}

	QString GetSizeAsString(const qreal factor) const override;

	void Update() override;

	void Draw(QPainter* painter) const override;

private:
	QRectF rect_;
};

class Circle : public Shape
{
public:
	Circle() : Shape(Shape::Type::CIRCLE) {}

	Circle(const QPen& pen) : Shape(Type::CIRCLE, 2, pen) {}

	Node* GetNextNode() override;

	Node* GetOrientationNode(Node* selectedNode) override;

	QString GetSizeAsString(const qreal factor) const override;

	void Update() override;

	void Draw(QPainter* painter) const override;

	void DrawHelpers(QPainter* painter) const override;

private:
	QRectF rect_;
	QLineF radius_;
};

class Oval : public Shape
{
public:
	Oval() : Shape(Shape::Type::OVAL) {}

	Oval(const QPen& pen) : Shape(Type::OVAL, 2, pen) {}

	QString GetSizeAsString(const qreal factor) const override;

	void Update() override;

	void Draw(QPainter* painter) const override;

	void DrawHelpers(QPainter* painter) const override;

private:
	QRectF rect_;
};

class Curve : public Shape
{
public:
	Curve() : Shape(Shape::Type::CURVE) {}

	Curve(const QPen& pen) : Shape(Type::CURVE, 3, pen) {}

	Node* GetOrientationNode(Node* selectedNode) override;

	void Update() override;

	void Draw(QPainter* painter) const override;

	void DrawHelpers(QPainter* painter) const override;

private:
	QPainterPath path_;
	QLineF helpLine1_;
	QLineF helpLine2_;
};

class Sector : public Shape
{
public:
	Sector() : Shape(Shape::Type::SECTOR) {}

	Sector(const QPen& pen) : Shape(Type::SECTOR, 3, pen) {}

	Node* GetNextNode() override;

	QString GetSizeAsString(const qreal factor) const override;

	void Update() override;

	void Draw(QPainter* painter) const override;

private:
	QRectF rect_;
	qint32 startAngle_{ 0 };
	qint32 sectorAngle_{ 0 };
};