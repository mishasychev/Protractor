#pragma once

#include <QToolButton>

class LinePattern : public QToolButton
{
	Q_OBJECT

public:
	LinePattern(QWidget* parent) : QToolButton(parent) {}

protected:
	void paintEvent(QPaintEvent* event) override;

private:
	QList<qreal> pattern_;

public:
	__forceinline void SetPattern(QList<qreal>&& pattern) { pattern_ = std::move(pattern); }
	__forceinline auto GetPattern() const { return pattern_; }
};

