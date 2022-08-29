#pragma once

#include <QDialog>
#include "ui_NodeLocationDialog.h"

#include <unordered_map>
#include <vector>

class Workspace;

class NodeLocationDialog : public QDialog, public Ui::NodeLocationDialog
{
	Q_OBJECT

	enum class Axis : quint8
	{
		X,
		Y,
		Z
	};

public:
	NodeLocationDialog(QWidget *parent);
	virtual ~NodeLocationDialog();

private:
	std::pair<Axis, Axis> GetAxes_() const;

	void FirstAxisChanged_(qint32 index) const;

public:
	Vector2D GetLocation(const Workspace* ws) const;
};
