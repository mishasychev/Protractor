#pragma once

#include <QDialog>
#include "ui_PrintPreparationDialog.h"

class Workspace;

class PrintPreparationDialog : public QDialog, public Ui::PrintPreparationDialog
{
	Q_OBJECT

public:
	PrintPreparationDialog(QWidget* parent, Workspace* ws);

	Vector2D GetOffset() const { return viewer->GetOffset(); }

	bool ShouldPrintFrame() const { return frameCheckBox->checkState() == Qt::Checked; }

private:
	void InitializeActions_();

	void OnFrameCheckBoxStateChanged(const qint32 newState) const;
};
