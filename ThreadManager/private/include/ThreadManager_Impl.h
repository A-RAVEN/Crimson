#pragma once
#include <header/ThreadManager.h>
#include "ThreadSafePointerPool.h"
#include <deque>

namespace thread_management
{
	class CTaskGraph_Impl;
	class CThreadManager_Impl;

	class CTask_Impl : public CTask
	{
	public:
		CTask_Impl(CTaskGraph_Impl& owningGraph);
		// 通过 CTask 继承
		virtual CTask* Succeed(CTask* parentTask) override;
		virtual CTask* Name(std::string name) override;
		virtual CTask* Functor(std::function<void()> functor) override;
		void SetupWaitingForCounter();
		void Invoke();
		void TryDecCounter();
	private:
		std::function<void()> m_Functor;
		std::string m_Name;
		std::shared_ptr<CTaskGraph> p_TaskGraph;
		std::vector<CTask_Impl*>m_Dependents;
		std::vector<CTask_Impl*>m_Successors;
		CTaskGraph_Impl& m_OwningGraph;
		friend class CTaskGraph_Impl;
		std::atomic<uint32_t>m_PendingTaskCount{0};
	};


	class CTaskGraph_Impl : public CTaskGraph
	{
	public:
		CTaskGraph_Impl(CThreadManager_Impl& owningManager);
		void ReleaseGraph();
		// 通过 CTaskGraph 继承
		virtual CTask* NewTask() override;
		virtual CTaskGraph* Name(std::string name) override;
		virtual CTaskGraph* FinalizeFunctor(std::function<void()> functor) override;

		void SetupTopology();
		CThreadManager_Impl& GetThreadMaanger() const { return m_OwningManager; }
		void TryDecCounter();
	private:
		std::function<void()> m_Functor;
		std::string m_Name;
		std::deque<CTask_Impl> m_Tasks;
		std::vector<CTask_Impl*> m_SourceTasks;
		CThreadManager_Impl& m_OwningManager;
		std::atomic<uint32_t>m_PendingTaskCount{0};
		friend class CThreadManager_Impl;
	};

	class CThreadManager_Impl : public CThreadManager
	{
	public:
		CThreadManager_Impl();
		// 通过 CThreadManager 继承
		virtual void InitializeThreadCount(uint32_t threadNum) override;
		virtual std::future<int> EnqueueAnyThreadWorkWithPromise(std::function<void()> function) override;
		virtual void ExecuteTaskGraph(CTaskGraph* graph) override;
		virtual CTaskGraph* NewTaskGraph() override;

		void RemoveTaskGraph(CTaskGraph_Impl* graph);
		void EnqueueGraphTask(CTask_Impl* newTask);
		void EnqueueGraphTasks(std::vector<CTask_Impl*> const& tasks);
	private:
		void ProcessingWorks();
	private:
		std::deque<CTask_Impl*> m_TaskQueue;
		bool m_Stopped = false;
		std::vector<std::thread> m_WorkerThreads;
		std::mutex m_Mutex;
		std::condition_variable m_ConditinalVariable;

		threadmanager_utils::TThreadSafePointerPool<CTaskGraph_Impl> m_TaskGraphPool;
	};
}