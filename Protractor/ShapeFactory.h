#pragma once

#include <QPen>
#include <memory>

class Shape;

class IShapeFactory
{
public:
	[[nodiscard]] virtual std::unique_ptr<Shape> Create(const QPen& pen) const = 0;
};

template<typename T>
concept ShapeDerived = std::is_base_of_v<Shape, T>;

template<ShapeDerived T>
class ShapeFactory : public IShapeFactory
{
public:
	[[nodiscard]] std::unique_ptr<Shape> Create(const QPen& pen) const override;
};

template<ShapeDerived T>
std::unique_ptr<Shape> ShapeFactory<T>::Create(const QPen& pen) const
{
	return std::make_unique<T>(pen);
}
