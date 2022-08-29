#include "stdafx.h"

#include "MainWindow.h"

#include <QLabel>
#include <QMouseEvent>

#include <QPrintDialog>
#include <QPrinter>

#include "Workspace.h"
#include "NodeSearcher.h"
#include "Shape.h"
#include "ShapeFactory.h"
#include "WorkspaceSettings.h"
#include "PrintPreparationDialog.h"
#include "DoubleSpinLabel.h"
#include "LinePattern.h"
#include "NodeLocationDialog.h"

#include <functional>
#include <ranges>

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent),
	printer_(new QPrinter(QPrinter::HighResolution)),
	nodeSearcher_(new NodeSearcher()),
	shapeSelectionGroup_(new QActionGroup(this)),
	coordinateLabel_(new QLabel(this)),
	shapeInfoLabel_(new QLabel(this)),
	thicknessSpinLabel_(new DoubleSpinLabel(this, 0.25, 0.05, 0.1, 2.0)),
	patternsMainButton_(new LinePattern(this))
{
	setupUi(this);

	InitializeActions_();

	//Init documentTabs
	DeleteWorkspace_(0);
	AddWorkspace_(new Workspace(documentTabs, FormatType::A4, nodeSearcher_.get()));

	//Init Shape's action group
	shapeSelectionGroup_->setExclusive(true);
	shapeSelectionGroup_->addAction(actionLine);
	shapeSelectionGroup_->addAction(actionBox);
	shapeSelectionGroup_->addAction(actionCircle);
	shapeSelectionGroup_->addAction(actionEllipse);
	shapeSelectionGroup_->addAction(actionCurve);
	shapeSelectionGroup_->addAction(actionSector);

	//Init status bar
	mainStatusBar->addWidget(coordinateLabel_.get(), 1);
	mainStatusBar->addWidget(shapeInfoLabel_.get(), 1);

	//Init pen thickness label
	thicknessSpinLabel_->SetValue(1.0);
	thicknessSpinLabel_->SetPrefix(QString("Thickness: "));
	thicknessSpinLabel_->SetSuffix(QString("mm  "));
	lineSettingsToolBar->addWidget(thicknessSpinLabel_.get());

	//Init pen patterns selection
	auto* patternsMenu = new QMenu(patternsMainButton_.get());
	patternsMainButton_->setMinimumSize(180, 30);
	patternsMainButton_->setMenu(patternsMenu);
	patternsMainButton_->setPopupMode(QToolButton::InstantPopup);

	auto MakePattern = [&](QList<qreal>&& patternList)
	{
		auto* pattern = new LinePattern(patternsMainButton_.get());
		pattern->setMinimumSize(180, 30);
		pattern->SetPattern(std::move(patternList));

		auto* patternAction = new QWidgetAction(patternsMainButton_.get());
		patternAction->setDefaultWidget(pattern);

		connect(pattern, &LinePattern::pressed, std::bind(&MainWindow::OnLinePatternButtonPressed_, this, pattern));

		patternsMenu->addAction(patternAction);
	};

	MakePattern({});
	MakePattern({ 8.0, 8.0 });
	MakePattern({ 15.0, 4.0, 2.0, 4.0 });

	lineSettingsToolBar->addWidget(patternsMainButton_.get());
}

MainWindow::~MainWindow() { }


void MainWindow::InitializeFromFile(const QString& path) const
{
	DeleteWorkspace_(0);
	OpenFile(path);
}

bool MainWindow::SaveFile(const QString& path) const
{
	QFile file(path);
	if (!file.open(QIODevice::WriteOnly))
		return false;

	QDataStream out(&file);
	out.setVersion(QDataStream::Qt_6_2);

	auto* ws = GetCurrentWorkspace();
	ws->Serialize(out);
	ws->SetFilePath(path);

	file.close();

	return true;
}

bool MainWindow::OpenFile(const QString& path) const
{
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly))
		return false;

	QDataStream in(&file);
	in.setVersion(QDataStream::Qt_6_2);

	const auto newWorkspace = new Workspace(documentTabs, FormatType::A3, nodeSearcher_.get());
	newWorkspace->Deserialize(in);
	newWorkspace->SetFilePath(path);

	AddWorkspace_(newWorkspace);

	file.close();

	return true;
}

QPen MainWindow::GetPen() const
{
	const auto* wsSettings = WorkspaceSettings::Instance();
	const auto factor = wsSettings->GetFactor(GetCurrentWorkspace()->GetFormatType());
	const auto userWidth = thicknessSpinLabel_->GetValue();

	QPen result;
	result.setWidthF(userWidth / factor);
	result.setCapStyle(Qt::RoundCap);
	
	auto pattern = patternsMainButton_->GetPattern();
	for (auto& n : pattern)
		n /= userWidth;
	result.setDashPattern(pattern);

	return result;
}

Workspace* MainWindow::GetCurrentWorkspace() const
{
	return dynamic_cast<Workspace*>(documentTabs->currentWidget());
}

void MainWindow::InitializeActions_()
{
	//File Menu
	connect(actionNewA3File, &QAction::triggered, this, &MainWindow::OnNewA3File_);
	connect(actionNewA4File, &QAction::triggered, this, &MainWindow::OnNewA4File_);
	connect(actionOpen, &QAction::triggered, this, &MainWindow::OnOpenFile_);
	connect(actionSave, &QAction::triggered, this, &MainWindow::OnSaveFile_);
	connect(actionSave_as, &QAction::triggered, this, &MainWindow::OnSaveFileAs_);
	connect(actionPrint, &QAction::triggered, this, &MainWindow::OnPrintFile_);

	//View Menu
	connect(actionReset_Transform, &QAction::triggered, this, &MainWindow::OnResetTransform_);

	//Shape Menu
	connect(actionLine, &QAction::triggered, this, &MainWindow::OnNewShape_<Line>);
	connect(actionBox, &QAction::triggered, this, &MainWindow::OnNewShape_<Box>);
	connect(actionCircle, &QAction::triggered, this, &MainWindow::OnNewShape_<Circle>);
	connect(actionEllipse, &QAction::triggered, this, &MainWindow::OnNewShape_<Oval>);
	connect(actionCurve, &QAction::triggered, this, &MainWindow::OnNewShape_<Curve>);
	connect(actionSector, &QAction::triggered, this, &MainWindow::OnNewShape_<Sector>);

	//Node Location Dialog
	connect(actionSet_Node_Location, &QAction::triggered, this, &MainWindow::OnNodeLocation_);

	//Tabs
	connect(documentTabs, &QTabWidget::tabCloseRequested, this, &MainWindow::DeleteWorkspace_);
	connect(documentTabs, &QTabWidget::currentChanged, this, &MainWindow::OnChangeCurrentTab_);
}

template <typename T>
void MainWindow::OnNewShape_() const
{
	GetCurrentWorkspace()->SetShapeFactory(new ShapeFactory<T>());
}

void MainWindow::UpdateActions_() const
{
	const bool bIsWsValid = GetCurrentWorkspace() != nullptr;

	//File Menu
	actionSave->setEnabled(bIsWsValid);
	actionSave_as->setEnabled(bIsWsValid);
	actionPrint->setEnabled(bIsWsValid);

	//View Menu
	actionReset_Transform->setEnabled(bIsWsValid);

	//Shape Menu
	actionLine->setEnabled(bIsWsValid);
	actionLine->setChecked(true);
	actionBox->setEnabled(bIsWsValid);
	actionCircle->setEnabled(bIsWsValid);
	actionEllipse->setEnabled(bIsWsValid);
	actionCurve->setEnabled(bIsWsValid);
	actionSector->setEnabled(bIsWsValid);

	//Node Location Dialog
	actionSet_Node_Location->setEnabled(bIsWsValid);
}

void MainWindow::OnNewA3File_() const
{
	AddWorkspace_(new Workspace(documentTabs, FormatType::A3, nodeSearcher_.get()));
}

void MainWindow::OnNewA4File_() const
{
	AddWorkspace_(new Workspace(documentTabs, FormatType::A4, nodeSearcher_.get()));
}

void MainWindow::OnOpenFile_()
{
	const auto filePath = QFileDialog::getOpenFileName(this, tr("Open Protractor Blueprint"), "", tr("Protractor Blueprint (*.prob)"));
	if (!OpenFile(filePath))
	{
		QMessageBox::critical(this, "Opening error", "This blueprint cannot be opened");
	}
}

void MainWindow::OnSaveFile_()
{
	const auto& filePath = GetCurrentWorkspace()->GetFilePath();

	if (filePath.isEmpty())
		OnSaveFileAs_();

	if(!SaveFile(filePath))
	{
		QMessageBox::critical(this, "Saving error", "This blueprint cannot be saved");
	}
}

void MainWindow::OnSaveFileAs_()
{
	const auto filePath = QFileDialog::getSaveFileName(this, tr("Save Protractor Blueprint"), "", tr("Protractor Blueprint (*.prob)"));
	if (SaveFile(filePath))
		documentTabs->setTabText(documentTabs->currentIndex(), GetFixedTabTitle_(GetCurrentWorkspace()));
}

void MainWindow::OnPrintFile_()
{
	auto* currentWS = GetCurrentWorkspace();

	PrintPreparationDialog d(this, currentWS);
	if (d.exec() == QDialog::Rejected)
		return;

	QPrintDialog dialog(printer_.get(), this);
	dialog.setWindowTitle(tr("Print Protractor Blueprint"));

	if (dialog.exec() == QDialog::Rejected)
		return;

	QPainter painter(printer_.get());
	painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

	const auto* wsSettings = WorkspaceSettings::Instance();
	const auto scale = printer_->paperRect(QPrinter::DevicePixel).width() / wsSettings->GetMaxWorkspaceSize().x;
	painter.translate(d.GetOffset() * scale);
	painter.scale(scale, scale);
	currentWS->DrawShapes(&painter);

	if (d.ShouldPrintFrame())
	{
		const auto frameScale = printer_->paperRect(QPrinter::DevicePixel).width() / wsSettings->GetFormatSizeByType(currentWS->GetFormatType()).x;
		painter.resetTransform();
		painter.scale(frameScale, frameScale);
		currentWS->DrawFrame(&painter);
	}
}

void MainWindow::OnResetTransform_() const
{
	GetCurrentWorkspace()->ResetTransform();
}

void MainWindow::OnNodeLocation_()
{
	auto* ws = GetCurrentWorkspace();
	auto* node = GetCurrentWorkspace()->GetCurrentNode();

	NodeLocationDialog d(this);

	if ((node == nullptr) || (d.exec() == QDialog::Rejected))
		return;

	node->position = d.GetLocation(ws);
	node->parent->Update();
	QMouseEvent event(QEvent::MouseButtonRelease, 
		QPointF(), 
		Qt::MouseButton::LeftButton, 
		Qt::MouseButton::LeftButton, 
		Qt::KeyboardModifier::NoModifier);
	QCoreApplication::sendEvent(ws, &event);
}

void MainWindow::OnLinePatternButtonPressed_(LinePattern* pattern) const
{
	if (pattern == nullptr)
		return;

	patternsMainButton_->SetPattern(pattern->GetPattern());
	patternsMainButton_->menu()->hide();
	pattern->setDown(false);
}

void MainWindow::OnChangeCurrentTab_(const qint32 index) const
{
	if (index == -1)
		return;

	auto* ws = dynamic_cast<Workspace*>(documentTabs->widget(index));
	ws->SetNodeSearcher(nodeSearcher_.get());
	nodeSearcher_->SetWorkspace(ws);
}

void MainWindow::AddWorkspace_(Workspace* newWorkspace) const
{
	documentTabs->addTab(newWorkspace, GetFixedTabTitle_(newWorkspace));
	documentTabs->setCurrentWidget(newWorkspace);
	newWorkspace->setFocus();
	nodeSearcher_->SetWorkspace(newWorkspace);

	UpdateActions_();
}

void MainWindow::DeleteWorkspace_(const qint32 index) const
{
	const QWidget* oldWidget{ documentTabs->widget(index) };
	documentTabs->removeTab(index);
	delete oldWidget;

	UpdateActions_();
}

QString MainWindow::GetFixedTabTitle_(const Workspace* ws) const
{
	const QFileInfo fileInfo(ws->GetFilePath());

	auto newTabName = fileInfo.baseName();

	if (newTabName.isEmpty())
		return QString("New Tab %0").arg(documentTabs->count() + 1);

	return newTabName;
}
