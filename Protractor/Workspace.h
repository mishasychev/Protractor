// ReSharper disable All
#pragma once

#include <QWidget>
#include <memory>
#include <vector>
#include <array>
#include <atomic>
#include <QColor>
#include <QImage>
#include <tuple>

#include "ShapeFactory.h"
#include "WorkspaceSettings.h"

class Node;
class Shape;
class NodeSearcher;

class Workspace : public QWidget
{
	Q_OBJECT

public:
	friend class NodeSearcher;

	Workspace(QWidget* parent, const FormatType type, NodeSearcher* nodeSearcher);
	virtual ~Workspace();

	void ResetTransform();

	void Serialize(QDataStream& out) const;
	void Deserialize(QDataStream& in);

	void DrawShapes(QPainter* painter) const;
	void DrawFrame(QPainter* painter) const;

protected:
	void paintEvent(QPaintEvent* event) override;

	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

	void mouseMoveEvent(QMouseEvent* event) override;

	void wheelEvent(QWheelEvent* event) override;

	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

	void resizeEvent(QResizeEvent* event) override;

private:
	void DrawHelpers_(QPainter* painter) const;
	void DrawNodes_(QPainter* painter) const;
	void DrawHelperLines_(QPainter* painter) const;
	void DrawAxes_(QPainter* painter) const;

	void UpdateSpecialKeys_();
	void UpdateNodeOnLines_(Node* fromX, Node* fromY);
	void UpdateNearestNode_(const Node* nearestNode);
	void UpdateStraightLine_();

private:
	FormatType type_;

	//All shapes are stored here
	std::vector<std::unique_ptr<Shape>> shapes_;

	std::unique_ptr<Shape> selectedShape_;
	Node* selectedNode_;
	Vector2D targetPos_;

	Vector2D offset_;

	Vector2D startPan_;

	qreal scale_;
	Vector2D currentSize_;

	QString filePath_;

	std::unique_ptr<IShapeFactory> shapeFactory_;

	NodeSearcher* nodeSearcher_;

	bool bIsCtrlPressed_;
	bool bIsShiftPressed_;
	bool bIsAltPressed_;

	std::pair<Node*, Node*> nodesOnLines_;

public:
	__forceinline Vector2D WorldToScreen(const Vector2D& world) const { return (world + offset_) * scale_; }
	__forceinline Vector2D ScreenToWorld(const Vector2D& screen) const { return (screen / scale_) - offset_; }

	__forceinline Node* GetCurrentNode() const { return selectedNode_; }

	__forceinline FormatType GetFormatType() const { return type_; }

	__forceinline qreal GetScale() const { return scale_; }
	__forceinline void SetScale(const qreal newScale) { scale_ = newScale; }

	__forceinline Vector2D GetSize() const { return currentSize_; }

	__forceinline void SetFilePath(const QString& newFilePath) { filePath_ = newFilePath; }
	__forceinline const QString& GetFilePath() const { return filePath_; }

	__forceinline IShapeFactory* GetShapeFactory() const { return shapeFactory_.get(); }
	__forceinline void SetShapeFactory(IShapeFactory* newCreator) { shapeFactory_.reset(newCreator); }

	__forceinline void SetNodeSearcher(NodeSearcher* searcher) { nodeSearcher_ = searcher; }

	QString GetTargetPositionAsString() const;

	QString GetSelectedShapeInfoAsString() const;

	//States
private:
	enum class State : quint8
	{
		NONE,
		SHAPE_MODIFICATION,

		CTRL_BUTTON_PRESSED,
		SHIFT_BUTTON_PRESSED
	};

	class StateHandler
	{
		template<typename T>
		using MethodPtr = void(Workspace::*)(const T*);

	public:
		MethodPtr<QMouseEvent> OnMouseRelease{ nullptr };
		MethodPtr<QMouseEvent> OnMouseMove{ nullptr };
	};

	constexpr void InitializeStates_();

	void ST_NONE_OnMouseRelease_(const QMouseEvent* event);

	void ST_SHAPE_MODIFICATION_OnMouseRelease_(const QMouseEvent* event);
	void ST_SHAPE_MODIFICATION_OnMouseMove_(const QMouseEvent* event);

	State currentState_;
	std::array<StateHandler, 2> states_;
};