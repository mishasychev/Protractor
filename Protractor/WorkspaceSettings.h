#pragma once

#include "Vector2D.h"

enum class FormatType : uint8_t
{
	A3,
	A4
};

class WorkspaceSettings
{
	WorkspaceSettings() {};

public:
	static WorkspaceSettings* Instance();

	static inline constexpr Vector2D A3Size{ 420.0, 297.0 };
	static inline constexpr Vector2D A4Size{ 210.0, 297.0 };

private:
	Vector2D maxWorkspaceSize_;

public:
	Vector2D GetMaxWorkspaceSize() const { return maxWorkspaceSize_; }
	void SetMaxWorkspaceSize(const Vector2D& newSize) { maxWorkspaceSize_ = newSize; }

	Vector2D GetFormatSizeByType(const FormatType type) const;

	double GetFactor(const FormatType type) const;
};