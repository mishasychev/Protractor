#include "stdafx.h"

#include "NodeLocationDialog.h"

#include "Workspace.h"
#include "WorkspaceSettings.h"

NodeLocationDialog::NodeLocationDialog(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
	setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);

	firstCoordinateBox->addItem("X:");
	firstCoordinateBox->addItem("Y:");
	connect(firstCoordinateBox, &QComboBox::currentIndexChanged, this, &NodeLocationDialog::FirstAxisChanged_);

	secondCoordinateBox->addItem("Y:");
	secondCoordinateBox->addItem("Z:");

	connect(applyButton, &QPushButton::pressed, [this] { done(QDialog::Accepted); });
	connect(exitButton, &QPushButton::pressed, [this] { done(QDialog::Rejected); });
}

NodeLocationDialog::~NodeLocationDialog() {}

std::pair<NodeLocationDialog::Axis, NodeLocationDialog::Axis> NodeLocationDialog::GetAxes_() const
{
	return std::make_pair(static_cast<Axis>(firstCoordinateBox->currentIndex()), 
		static_cast<Axis>(secondCoordinateBox->currentIndex() + 1));
}

void NodeLocationDialog::FirstAxisChanged_(qint32 index) const
{
	auto* model = dynamic_cast<QStandardItemModel*>(secondCoordinateBox->model());
	if (model == nullptr)
		return;

	const auto firstAxis = static_cast<Axis>(firstCoordinateBox->currentIndex());

	const auto fChangeItemVisibility = [&](const qint32 ind, const bool newVisibility)
	{
		auto* item = model->item(ind);
		item->setEnabled(newVisibility);
	};

	switch (firstAxis)
	{
	case Axis::X:
		fChangeItemVisibility(0, true);
		fChangeItemVisibility(1, true);
		break;
	case Axis::Y:
		secondCoordinateBox->setCurrentIndex(1);
		fChangeItemVisibility(0, false);
		break;
	}
}

Vector2D NodeLocationDialog::GetLocation(const Workspace* ws) const
{
	const auto* wsSettings = WorkspaceSettings::Instance();

	const auto factor = wsSettings->GetFactor(ws->GetFormatType());
	const auto centre = wsSettings->GetMaxWorkspaceSize() / 2.0;

	const auto [f, s] = GetAxes_();

	Vector2D userPos(firstCoordinateEdit->value(), secondCoordinateEdit->value());
	userPos /= factor;

	if (f == Axis::X && s == Axis::Y)
		return { centre.x - userPos.x, centre.y + userPos.y };
	if (f == Axis::X && s == Axis::Z)
		return centre - userPos;
	if (f == Axis::Y && s == Axis::Z)
		return { centre.x + userPos.x, centre.y - userPos.y };

	return Vector2D{};
}
