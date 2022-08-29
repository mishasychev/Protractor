#include "stdafx.h"

#include "WorkspaceSettings.h"

WorkspaceSettings* WorkspaceSettings::Instance()
{
	static WorkspaceSettings wsSettings;
	return &wsSettings;
}

Vector2D WorkspaceSettings::GetFormatSizeByType(const FormatType type) const
{
	switch (type)
	{
	case FormatType::A3: return A3Size;
	case FormatType::A4: return A4Size;
	default: return Vector2D();
	}
}

double WorkspaceSettings::GetFactor(const FormatType type) const
{
	return GetFormatSizeByType(type).x / maxWorkspaceSize_.x;
}
