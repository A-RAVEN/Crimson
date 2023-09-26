#pragma once
#include <functional>
#include <future>

namespace thread_management
{
	class CThreadManager;
	class CTaskGraph;
	class TaskParallelFor;
	class CTask
	{
	public:
		virtual ~CTask() = default;
		CTask() = default;
		CTask(CTask const& other) = delete;
		CTask& operator=(CTask const& other) = delete;
		CTask(CTask&& other) = delete;
		CTask& operator=(CTask&& other) = delete;

		virtual CTask* Name(std::string name) = 0;
		virtual CTask* DependsOn(CTask* parentTask) = 0;
		virtual CTask* DependsOn(TaskParallelFor* parentTask) = 0;
		virtual CTask* DependsOn(CTaskGraph* parentTask) = 0;
		virtual std::shared_future<void> Run() = 0;

		virtual CTask* Functor(std::function<void()> functor) = 0;
	};

	class TaskParallelFor
	{
	public:
		virtual ~TaskParallelFor() = default;
		TaskParallelFor() = default;
		TaskParallelFor(TaskParallelFor const& other) = delete;
		TaskParallelFor& operator=(TaskParallelFor const& other) = delete;
		TaskParallelFor(TaskParallelFor&& other) = delete;
		TaskParallelFor& operator=(TaskParallelFor&& other) = delete;

		virtual TaskParallelFor* Name(std::string name) = 0;
		virtual TaskParallelFor* DependsOn(CTask* parentTask) = 0;
		virtual TaskParallelFor* DependsOn(TaskParallelFor* parentTask) = 0;
		virtual TaskParallelFor* DependsOn(CTaskGraph* parentTask) = 0;

		virtual TaskParallelFor* Functor(std::function<void(uint32_t)> functor) = 0;
		virtual TaskParallelFor* JobCount(uint32_t jobCount) = 0;
		virtual std::shared_future<void> Run() = 0;
	};

	class CTaskGraph
	{
	public:
		virtual ~CTaskGraph() = default;
		CTaskGraph() = default;
		CTaskGraph(CTaskGraph const& other) = delete;
		CTaskGraph& operator=(CTaskGraph const& other) = delete;
		CTaskGraph(CTaskGraph && other) = delete;
		CTaskGraph& operator=(CTaskGraph && other) = delete;

		virtual CTaskGraph* Name(std::string name) = 0;
		virtual CTaskGraph* DependsOn(CTask* parentTask) = 0;
		virtual CTaskGraph* DependsOn(TaskParallelFor* parentTask) = 0;
		virtual CTaskGraph* DependsOn(CTaskGraph* parentTask) = 0;
		//ÑÓ³Ù³õÊ¼»¯º¯Êý
		virtual CTaskGraph* SetupFunctor(std::function<void(CTaskGraph* thisGraph)> functor) = 0;
		virtual std::shared_future<void> Run() = 0;

		virtual CTask* NewTask() = 0;
		virtual TaskParallelFor* NewTaskParallelFor() = 0;
		virtual CTaskGraph* NewTaskGraph() = 0;
	};

	class CThreadManager
	{
	public:
		virtual ~CThreadManager() = default;
		CThreadManager() = default;
		CThreadManager(CThreadManager const& other) = delete;
		CThreadManager& operator=(CThreadManager const& other) = delete;
		CThreadManager(CThreadManager&& other) = delete;
		CThreadManager& operator=(CThreadManager&& other) = delete;

		virtual void InitializeThreadCount(uint32_t threadNum) = 0;
		virtual CTask* NewTask() = 0;
		virtual TaskParallelFor* NewTaskParallelFor() = 0;
		virtual CTaskGraph* NewTaskGraph() = 0;
	};
}