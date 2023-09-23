#pragma once
#include <future>
#include <deque>
#include <unordered_map>
#include <header/ThreadManager.h>
#include <SharedTools/header/ThreadSafePool.h>
#include "TaskNode.h"

namespace thread_management
{
	class CTask_Impl1 : public TaskNode, public CTask
	{
	public:
		virtual CTask* Name(std::string name) override;
		virtual CTask* DependsOn(CTask* parentTask) override;
		virtual CTask* DependsOn(TaskParallelFor* parentTask) override;
		virtual CTask* DependsOn(CTaskGraph* parentTask) override;
		virtual std::shared_future<void> Run() override;

		virtual CTask* Functor(std::function<void()> functor) override;
	public:
		CTask_Impl1(TaskBaseObject* owner, ThreadManager_Impl1* owningManager);
		// 通过 CTask 继承
		 void Functor_Internal(std::function<void()> functor);
		 void Initialize() {}
		 void Release();
	private:
		std::function<void()> m_Functor;
		// 通过 TaskNode 继承
		virtual void Execute_Internal() override;
	};

	class TaskParallelFor_Impl : public TaskNode, public TaskParallelFor
	{
	public:
		virtual TaskParallelFor* Name(std::string name) override;
		virtual TaskParallelFor* DependsOn(CTask* parentTask) override;
		virtual TaskParallelFor* DependsOn(TaskParallelFor* parentTask) override;
		virtual TaskParallelFor* DependsOn(CTaskGraph* parentTask) override;
		virtual TaskParallelFor* Functor(std::function<void(uint32_t)> functor) override;
		virtual std::shared_future<void> Dispatch(uint32_t jobCount) override;

	public:
		TaskParallelFor_Impl(TaskBaseObject* owner, ThreadManager_Impl1* owningManager);

		void Initialize() {}
		void Release();

		// 通过 TaskNode 继承
		virtual void NotifyChildNodeFinish(TaskNode* childNode) override;
		virtual void Execute_Internal() override;
	private:
		std::function<void(uint32_t)> m_Functor;
		std::atomic<uint32_t>m_PendingSubnodeCount{0};
		std::deque<CTask_Impl1> m_TaskPool;
	};

	class TaskGraph_Impl1 : public TaskNode, public CTaskGraph
	{
	public:
		virtual CTaskGraph* Name(std::string name) override;
		virtual CTaskGraph* DependsOn(CTask* parentTask) override;
		virtual CTaskGraph* DependsOn(TaskParallelFor* parentTask) override;
		virtual CTaskGraph* DependsOn(CTaskGraph* parentTask) override;
		virtual std::shared_future<void> Run() override;

		virtual CTask* NewTask() override;
		virtual TaskParallelFor* NewTaskParallelFor() override;
		virtual CTaskGraph* NewTaskGraph() override;

	public:
		TaskGraph_Impl1(TaskBaseObject* owner, ThreadManager_Impl1* owningManager);
		void Initialize() {}
		void Release();
	protected:
		CTask_Impl1* NewTask_Internal();
		TaskGraph_Impl1* NewSubTaskGraph_Internal();
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
		std::deque<TaskParallelFor_Impl> m_TaskParallelForPool;
		//在类里面放自己的deque会不会有问题？
		std::deque<TaskGraph_Impl1*> m_TaskGraphPool;
	};

	class ThreadManager_Impl1 : public TaskBaseObject, public CThreadManager
	{
	public:
		virtual void InitializeThreadCount(uint32_t threadNum) override;
		virtual CTask* NewTask() override;
		virtual TaskParallelFor* NewTaskParallelFor() override;
		virtual CTaskGraph* NewTaskGraph() override;
	public:
		ThreadManager_Impl1();
		~ThreadManager_Impl1();
		void EnqueueTaskNode(TaskNode* node);
		void EnqueueTaskNodes(std::deque<TaskNode*> const& nodeDeque);
		virtual void NotifyChildNodeFinish(TaskNode* childNode) override;
	private:
		void ProcessingWorks(uint32_t threadId);
	private:
		//
		std::deque<TaskNode*> m_TaskQueue;
		std::vector<std::thread> m_WorkerThreads;
		std::atomic_bool m_Stopped = false;
		std::mutex m_Mutex;
		std::condition_variable m_ConditinalVariable;

		threadsafe_utils::TThreadSafePointerPool<CTask_Impl1> m_TaskPool;
		threadsafe_utils::TThreadSafePointerPool<TaskParallelFor_Impl> m_TaskParallelForPool;
		threadsafe_utils::TThreadSafePointerPool<TaskGraph_Impl1> m_TaskGraphPool;
	};
}