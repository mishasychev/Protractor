#pragma once

#include <QLabel>

class DoubleSpinLabel : public QLabel
{
	Q_OBJECT

public:
	DoubleSpinLabel(QWidget* parent, qreal wheelStep = 1.0, qreal mouseStep = 1.0, qreal minimum = 0.0, qreal maximum = 100.0);

protected:
	void wheelEvent(QWheelEvent* event) override;

	void mouseMoveEvent(QMouseEvent* event) override;

	void mousePressEvent(QMouseEvent* event) override;

private:
	void UpdateText_();

	void ChangeValue_(qreal value);

private:
	QString prefix_;
	QString suffix_;

	qreal value_;

	qreal wheelStep_;
	qreal mouseStep_;
	
	qreal minValue_;
	qreal maxValue_;

	Vector2D startPan_;

public:
	__forceinline void SetPrefix(QString&& prefix) { prefix_ = std::move(prefix); UpdateText_(); }
	__forceinline QString GetPrefix() const { return prefix_; }

	__forceinline void SetSuffix(QString&& suffix) { suffix_ = std::move(suffix); UpdateText_(); }
	__forceinline QString GetSuffix() const { return suffix_; }

	__forceinline void SetValue(qreal value) { value_ = value; UpdateText_(); }
	__forceinline qreal GetValue() const { return value_; }

	__forceinline void SetWheelStep(qreal step) { wheelStep_ = step; }
	__forceinline qreal GetWheelStep() const { return wheelStep_; }

	__forceinline void SetMouseStep(qreal step) { mouseStep_ = step; }
	__forceinline qreal GetMouseStep() const { return mouseStep_; }

	__forceinline void SetMinimum(qreal minimum) { minValue_ = minimum; }
	__forceinline qreal GetMinimum() const { return minValue_; }

	__forceinline void SetMaximum(qreal maximum) { maxValue_ = maximum; }
	__forceinline qreal GetMaximum() const { return maxValue_; }
};

