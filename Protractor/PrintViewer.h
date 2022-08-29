#pragma once

#include <QWidget>

class Workspace;

class PrintViewer : public QWidget
{
	Q_OBJECT

public:
	PrintViewer(QWidget* parent);

	void Initialize();

protected:
	void paintEvent(QPaintEvent* event) override;

	void mousePressEvent(QMouseEvent* event) override;

	void mouseMoveEvent(QMouseEvent* event) override;

private:
	Workspace* ws_;

	Vector2D offset_;
	Vector2D startPan_;

	double scale_;

	bool isFrameOn_;

public:
	void SetWorkspace(Workspace* ws) { ws_ = ws; };

	Vector2D GetOffset() const { return offset_; }

	void SetIsFrameOn(const bool value) { isFrameOn_ = value; }
};