#include "stdafx.h"

#include "NodeSearcher.h"

#include "Workspace.h"
#include "Shape.h"

NodeSearcher::NodeSearcher()
	: atomicWs_(nullptr),
	bNeedToShutDown_(false),
	bIsSearchActive_(false),
	thread_([&]()
		{
			while (!bNeedToShutDown_.load(std::memory_order_relaxed))
			{
				std::unique_lock lk(NodeSearchMutex);
				cv_.wait(lk, [this] { return bIsSearchActive_; });

				const auto* ws = atomicWs_.load(std::memory_order_relaxed);
				if (ws == nullptr)
					continue;

				Node* nearestNode = nullptr;
				qreal distanceToNearestNode = 1000000.0;

				Node* fromX = nullptr;
				Node* fromY = nullptr;
				Vector2D toLines(1000000.0);

				for (const auto& shape : ws->shapes_)
				{
					for (auto& node : shape->GetNodes())
					{
						const auto nodePosition = ws->WorldToScreen(node.position);
						const auto targetPosition = ws->targetPos_;
						const auto distanceToNode = Vector2D::Distance(nodePosition, targetPosition);

						if (distanceToNode < distanceToNearestNode)
						{
							nearestNode = &node;
							distanceToNearestNode = distanceToNode;
						}

						const auto toNode = targetPosition - nodePosition;
						if (toNode.Abs().x < toLines.Abs().x)
						{
							toLines.x = toNode.x;
							fromX = &node;
						}
						if (toNode.Abs().y < toLines.Abs().y)
						{
							toLines.y = toNode.y;
							fromY = &node;
						}
					}
				}

				std::make_tuple(nearestNode, fromX, fromY).swap(latestSearchResult_);

				bIsSearchActive_ = false;
			}
		})
{
}
		NodeSearcher::~NodeSearcher()
		{
			bNeedToShutDown_.store(true, std::memory_order::memory_order_relaxed);
			Run();

			thread_.join();
		}

		void NodeSearcher::Run()
		{
			if (atomicWs_ == nullptr || bIsSearchActive_)
				return;

			std::unique_lock lk(NodeSearchMutex);
			bIsSearchActive_ = true;
			cv_.notify_one();
		}

		NodeSearcher::SearchResult NodeSearcher::GetLatestSearchResult()
		{
			std::unique_lock lk(NodeSearchMutex/*, std::try_to_lock*/);

			SearchResult sr = { nullptr, nullptr, nullptr };
			sr.swap(latestSearchResult_);

			return sr;
		}