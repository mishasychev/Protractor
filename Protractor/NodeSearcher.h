#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <tuple>

class Workspace;
class Node;

class NodeSearcher
{
public:
	static inline std::mutex NodeSearchMutex;

	using SearchResult = std::tuple<Node*, Node*, Node*>;

	NodeSearcher();

	~NodeSearcher();

	void Run();

private:
	std::atomic<Workspace*> atomicWs_;
	SearchResult latestSearchResult_;

	std::atomic<bool> bNeedToShutDown_;
	bool bIsSearchActive_;
	std::condition_variable cv_;
	std::thread	thread_;
public:
	Workspace* GetWorkspace() const { return atomicWs_.load(std::memory_order_relaxed); }
	void SetWorkspace(Workspace* newWs) { atomicWs_.store(newWs, std::memory_order_relaxed); }

	SearchResult GetLatestSearchResult();
};