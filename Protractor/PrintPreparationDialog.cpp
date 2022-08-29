#include "stdafx.h"

#include "PrintPreparationDialog.h"

#include "Workspace.h"

PrintPreparationDialog::PrintPreparationDialog(QWidget* parent, Workspace* ws)
	: QDialog(parent)
{
	setupUi(this);
	setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);

	viewer->SetWorkspace(ws);
	viewer->Initialize();

	InitializeActions_();
}

void PrintPreparationDialog::InitializeActions_()
{
	connect(okButton, &QPushButton::pressed, [this] { done(QDialog::Accepted); });
	connect(exitButton, &QPushButton::pressed, [this] { done(QDialog::Rejected); });

	connect(frameCheckBox, &QCheckBox::stateChanged, this, &PrintPreparationDialog::OnFrameCheckBoxStateChanged);
}

void PrintPreparationDialog::OnFrameCheckBoxStateChanged(const qint32 newState) const
{
	viewer->SetIsFrameOn(newState == Qt::Checked);
	viewer->update();
}
