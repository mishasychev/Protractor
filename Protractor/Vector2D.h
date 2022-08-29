#pragma once

#include <cmath>
#include <QPoint>

class Vector2D
{
public:
	double x{ 0.0 };
	double y{ 0.0 };

	Vector2D() = default;
	Vector2D(Vector2D&&) = default;

	constexpr Vector2D(const double Quad) : x(Quad), y(Quad) {}
	constexpr Vector2D(const double X, const double Y) : x(X), y(Y) {}
	constexpr Vector2D(const int32_t Quad) : x(static_cast<double>(Quad)), y(x) {}
	constexpr Vector2D(const int32_t X, const int32_t Y) : x(static_cast<double>(X)), y(static_cast<double>(Y)) {}

	constexpr Vector2D(const Vector2D& V) : x(V.x), y(V.y) {}
	Vector2D& operator=(const Vector2D& V) { x = V.x; y = V.y; return *this; }

	constexpr Vector2D(const QPointF& p) : x(p.x()), y(p.y()) {}
	Vector2D& operator=(const QPointF& p) { x = p.x(); y = p.y(); return *this; }
	constexpr operator QPointF() const { return { x, y }; }
	constexpr QPointF ToQPointF() const { return { x, y }; }


	constexpr Vector2D operator+(const Vector2D& V) const { return { x + V.x, y + V.y }; }
	constexpr Vector2D operator-(const Vector2D& V) const { return { x - V.x, y - V.y }; }
	constexpr Vector2D operator*(const Vector2D& V) const { return { x * V.x, y * V.y }; }
	constexpr Vector2D operator/(const Vector2D& V) const { return { x / V.x, y / V.y }; }

	constexpr Vector2D operator+(const double value) const { return { x + value, y + value }; }
	constexpr Vector2D operator-(const double value) const { return { x - value, y - value }; }
	constexpr Vector2D operator*(const double value) const { return { x * value, y * value }; }
	constexpr Vector2D operator/(const double value) const { return { x / value, y / value }; }


	constexpr Vector2D& operator+=(const Vector2D& V) { x += V.x; y += V.y; return *this; }
	constexpr Vector2D& operator-=(const Vector2D& V) { x -= V.x; y -= V.y; return *this; }
	constexpr Vector2D& operator*=(const Vector2D& V) { x *= V.x; y *= V.y; return *this; }
	constexpr Vector2D& operator/=(const Vector2D& V) { x /= V.x; y /= V.y; return *this; }

	constexpr Vector2D& operator+=(const double value) { x += value; y += value; return *this; }
	constexpr Vector2D& operator-=(const double value) { x -= value; y -= value; return *this; }
	constexpr Vector2D& operator*=(const double value) { x *= value; y *= value; return *this; }
	constexpr Vector2D& operator/=(const double value) { x /= value; y /= value; return *this; }


	constexpr bool operator==(const Vector2D& V) const { return x == V.x && y == V.y; }
	constexpr bool operator!=(const Vector2D& V) const { return !(x == V.x && y == V.y); }

	constexpr Vector2D operator-() const { return { -x, -y }; }

	QDataStream& Serialize(QDataStream& out) const { out << x << y; return out; }
	QDataStream& Deserialize(QDataStream& in) { in >> x >> y; return in; }

	double Length() const { return std::sqrt(x * x + y * y); }
	constexpr double LengthSquared() const { return x * x + y * y; }

	Vector2D Rotate(double angle) const;

	constexpr Vector2D Abs() const { return { std::abs(x), std::abs(y) }; }

	static double Distance(const Vector2D& V1, const Vector2D& V2);
	static double DistSquared(const Vector2D& V1, const Vector2D& V2);

	static constexpr double DotProduct(const Vector2D& V1, const Vector2D& V2);

	static double Angle(const Vector2D& V1, const Vector2D& V2);
};

__forceinline QDataStream& operator<<(QDataStream& out, const Vector2D& V)
{
	return V.Serialize(out);
}

__forceinline QDataStream& operator>>(QDataStream& in, Vector2D& V)
{
	return V.Deserialize(in);
}

__forceinline Vector2D Vector2D::Rotate(const double angle) const
{
	return { x * std::cos(angle) - y * std::sin(angle),
			 x * std::sin(angle) + y * std::cos(angle) };
}

__forceinline double Vector2D::Distance(const Vector2D& V1, const Vector2D& V2)
{
	return std::sqrt(DistSquared(V1, V2));
}

__forceinline double Vector2D::DistSquared(const Vector2D& V1, const Vector2D& V2)
{
	return std::pow(V2.x - V1.x, 2) + std::pow(V2.y - V1.y, 2);
}

__forceinline constexpr double Vector2D::DotProduct(const Vector2D& V1, const Vector2D& V2)
{
	return V1.x * V2.x + V1.y * V2.y;
}

__forceinline double Vector2D::Angle(const Vector2D& V1, const Vector2D& V2)
{
	return std::acos(DotProduct(V1, V2) / (V1.Length() * V2.Length()));
}
