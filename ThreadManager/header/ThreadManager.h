#pragma once
#include <functional>
#include <future>

namespace thread_management
{
	class CTask
	{
	public:
		virtual ~CTask() = default;
		CTask() = default;
		CTask(CTask const& other) = delete;
		CTask& operator=(CTask const& other) = delete;
		CTask(CTask&& other) = default;
		CTask& operator=(CTask&& other) = default;
		virtual CTask* Succeed(CTask* parentTask) = 0;
		virtual CTask* Name(std::string name) = 0;
		virtual CTask* Functor(std::function<void()> functor) = 0;
	};

	class CTaskGraph
	{
	public:
		virtual ~CTaskGraph() = default;
		CTaskGraph() = default;
		CTaskGraph(CTaskGraph const& other) = delete;
		CTaskGraph& operator=(CTaskGraph const& other) = delete;
		CTaskGraph(CTaskGraph && other) = default;
		CTaskGraph& operator=(CTaskGraph && other) = default;
		virtual CTaskGraph* Name(std::string name) = 0;
		virtual CTask* NewTask() = 0;
	};

	class CThreadManager
	{
	public:
		virtual ~CThreadManager() = default;
		CThreadManager() = default;
		CThreadManager(CThreadManager const& other) = delete;
		CThreadManager& operator=(CThreadManager const& other) = delete;
		CThreadManager(CThreadManager&& other) = default;
		CThreadManager& operator=(CThreadManager&& other) = default;
		virtual void InitializeThreadCount(uint32_t threadNum) = 0;
		virtual std::future<int> EnqueueAnyThreadWorkWithPromise(std::function<void()> function) = 0;
		virtual void ExecuteTaskGraph(CTaskGraph* graph) = 0;
		virtual CTaskGraph* NewTaskGraph() = 0;
	};
}