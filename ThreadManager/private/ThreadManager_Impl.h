#pragma once
#include <header/ThreadManager.h>
#include <SharedTools/header/ThreadSafePool.h>
#include <deque>
#include "TaskNode.h"

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
		CThreadManager_Impl& GetThreadManager() const { return m_OwningManager; }
		void TryDecCounter();
	private:
		void Finalize() const;
	private:
		std::function<void()> m_Functor;
		std::string m_Name;
		std::deque<CTask_Impl> m_Tasks;
		std::vector<CTask_Impl*> m_SourceTasks;
		CThreadManager_Impl& m_OwningManager;
		std::atomic<uint32_t>m_PendingTaskCount{0};
		std::promise<void> m_Promise;
		friend class CThreadManager_Impl;
	};

	class CThreadManager_Impl : public CThreadManager
	{
	public:
		CThreadManager_Impl();
		// 通过 CThreadManager 继承
		virtual void InitializeThreadCount(uint32_t threadNum) override;
		virtual std::future<int> EnqueueAnyThreadWorkWithPromise(std::function<void()> function) override;
		virtual std::shared_future<void> ExecuteTaskGraph(CTaskGraph* graph) override;
		virtual CTaskGraph* NewTaskGraph() override;

		void RemoveTaskGraph(CTaskGraph_Impl* graph);
		void EnqueueGraphTask(CTask_Impl* newTask);
		void EnqueueGraphTasks(std::vector<CTask_Impl*> const& tasks);
	private:
		void ProcessingWorks(uint32_t threadId);
	private:
		std::deque<CTask_Impl*> m_TaskQueue;
		bool m_Stopped = false;
		std::vector<std::thread> m_WorkerThreads;
		std::mutex m_Mutex;
		std::condition_variable m_ConditinalVariable;
		threadsafe_utils::TThreadSafePointerPool<CTaskGraph_Impl> m_TaskGraphPool;
	};

	class CTask_Impl1 : public TaskNode
	{
	public:
		CTask_Impl1(TaskBaseObject* owner, ThreadManager_Impl1* owningManager);
		// 通过 CTask 继承
		 CTask_Impl1& DependsOn(TaskNode* dependsOnTaskNode);
		 CTask_Impl1& Name(std::string const& name);
		 CTask_Impl1& Functor(std::function<void()> functor);
		 void Initialize() {}
		 void Release();
	private:
		std::function<void()> m_Functor;
		// 通过 TaskNode 继承
		virtual void Execute_Internal() override;
	};

	class TaskGraph_Impl1 : public TaskNode
	{
	public:
		TaskGraph_Impl1(TaskBaseObject* owner, ThreadManager_Impl1* owningManager);
		void Initialize() {}
		void Release() {}
	protected:
		CTask_Impl1* NewTask();
		TaskGraph_Impl1* NewSubTaskGraph();
		// 通过 TaskNode 继承
		virtual void NotifyChildNodeFinish(TaskNode* childNode) override;
		virtual void Execute_Internal() override;
		virtual void SetupSubnodeDependencies() override;
	private:
		std::deque<TaskNode*> m_RootTasks;
		std::atomic<uint32_t>m_PendingSubnodeCount{0};

		std::mutex m_Mutex;
		std::deque<TaskNode*> m_SubTasks;
		std::deque<CTask_Impl1> m_TaskPool;
		//在类里面放自己的deque会不会有问题？
		std::deque<TaskGraph_Impl1> m_TaskGraphPool;
	};

	class ThreadManager_Impl1 : public TaskBaseObject
	{
	public:
		ThreadManager_Impl1();
		void InitializeThreadCount(uint32_t threadNum);

		CTask_Impl1* NewTask();
		TaskGraph_Impl1* NewSubTaskGraph();

		void EnqueueTaskNode(TaskNode* enqueueNode);
		void EnqueueTaskNodes(std::deque<TaskNode*> const& nodeDeque);
	private:
		void ProcessingWorks(uint32_t threadId);
	private:
		//
		std::deque<TaskNode*> m_TaskQueue;
		std::vector<std::thread> m_WorkerThreads;
		bool m_Stopped = false;
		std::mutex m_Mutex;
		std::condition_variable m_ConditinalVariable;

		threadsafe_utils::TThreadSafePointerPool<CTask_Impl1> m_TaskPool;
		//在类里面放自己的deque会不会有问题？
		threadsafe_utils::TThreadSafePointerPool<TaskGraph_Impl1> m_TaskGraphPool;
	};
}