#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"

#include <memory>

class QScrollBar;
class Workspace;
class QPrinter;
class NodeSearcher;
class QLabel;
class DoubleSpinLabel;
class LinePattern;

class MainWindow : public QMainWindow, public Ui::MainWindowClass
{
	Q_OBJECT

public:
	MainWindow(QWidget* parent = nullptr);
	virtual ~MainWindow();

	void InitializeFromFile(const QString& path) const;

	bool SaveFile(const QString& path) const;
	bool OpenFile(const QString& path) const;

	QPen GetPen() const;

private:
	void InitializeActions_();
	void UpdateActions_() const;

	void OnNewA3File_() const;
	void OnNewA4File_() const;
	void OnOpenFile_();
	void OnSaveFile_();
	void OnSaveFileAs_();
	void OnPrintFile_();

	void OnResetTransform_() const;

	template<typename T>
	void OnNewShape_() const;

	void OnNodeLocation_();

	void OnLinePatternButtonPressed_(LinePattern* pattern) const;

	void OnChangeCurrentTab_(qint32 index) const;

	void AddWorkspace_(Workspace* newWorkspace) const;
	void DeleteWorkspace_(qint32 index) const;

	QString GetFixedTabTitle_(const Workspace* ws) const;

private:
	std::unique_ptr<QPrinter> printer_;

	std::unique_ptr<NodeSearcher> nodeSearcher_;

	std::unique_ptr<QActionGroup> shapeSelectionGroup_;

	std::unique_ptr<QLabel> coordinateLabel_;

	std::unique_ptr<QLabel> shapeInfoLabel_;

	std::unique_ptr<DoubleSpinLabel> thicknessSpinLabel_;

	std::unique_ptr<LinePattern> patternsMainButton_;

public:
	Workspace* GetCurrentWorkspace() const;

	NodeSearcher* GetNodeSearcher() const { return nodeSearcher_.get(); }

	QLabel* GetCoordinateLabel() const { return coordinateLabel_.get(); }

	QLabel* GetShapeInfoLabel() const { return shapeInfoLabel_.get(); }
};
