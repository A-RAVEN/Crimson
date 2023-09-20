#include "pch.h"
#include "ThreadManager_Impl.h"

namespace thread_management
{
    /*CTask_Impl::CTask_Impl(CTaskGraph_Impl& owningGraph) : m_OwningGraph(owningGraph)
    {
    }
    CTask* CTask_Impl::Succeed(CTask* parentTask)
    {
        CTask_Impl* task = static_cast<CTask_Impl*>(parentTask);
        assert((&task->m_OwningGraph) == (&m_OwningGraph));
        task->m_Successors.push_back(this);
        m_Dependents.push_back(task);
        return this;
    }

    CTask* CTask_Impl::Name(std::string name)
    {
        m_Name = name;
        return this;
    }

    CTask* CTask_Impl::Functor(std::function<void()> functor)
    {
        m_Functor = functor;
        return this;
    }
    void CTask_Impl::SetupWaitingForCounter()
    {
        uint32_t pendingCount = m_Dependents.size();
        m_PendingTaskCount.store(pendingCount, std::memory_order_relaxed);
    }
    void CTask_Impl::Invoke()
    {
        if (m_Functor != nullptr)
        {
            m_Functor();
        }
        std::atomic_thread_fence(std::memory_order_release);
        for (auto itrSuccessor = m_Successors.begin(); itrSuccessor != m_Successors.end(); ++itrSuccessor)
        {
            (*itrSuccessor)->TryDecCounter();
        }
        m_OwningGraph.TryDecCounter();
    }
    void CTask_Impl::TryDecCounter()
    {
        uint32_t remainCounter = --m_PendingTaskCount;
        std::atomic_thread_fence(std::memory_order_acquire);
        if (remainCounter == 0)
        {
            m_OwningGraph.GetThreadManager().EnqueueGraphTask(this);
        }
    }
    CTaskGraph_Impl::CTaskGraph_Impl(CThreadManager_Impl& owningManager) : m_OwningManager(owningManager)
    {
    }

    void CTaskGraph_Impl::ReleaseGraph()
    {
        m_Promise = std::promise<void>();
        m_Name = "";
        m_Tasks.clear();
        m_SourceTasks.clear();
        m_PendingTaskCount.store(0u, std::memory_order_relaxed);
    }

    CTask* CTaskGraph_Impl::NewTask()
    {
        m_Tasks.emplace_back(*this);
        return &m_Tasks.back();
    }
    CTaskGraph* CTaskGraph_Impl::Name(std::string name)
    {
        m_Name = name;
        return this;
    }
    CTaskGraph* CTaskGraph_Impl::FinalizeFunctor(std::function<void()> functor)
    {
        m_Functor = functor;
        return this;
    }
    void CTaskGraph_Impl::SetupTopology()
    {
        m_SourceTasks.clear();
        for (auto itr_Task = m_Tasks.begin(); itr_Task != m_Tasks.end(); ++itr_Task)
        {
            itr_Task->SetupWaitingForCounter();
            if (itr_Task->m_Dependents.empty())
            {
                m_SourceTasks.push_back(&*itr_Task);
            }
        }
        uint32_t pendingTaskCount = m_Tasks.size();
        m_PendingTaskCount.store(pendingTaskCount, std::memory_order_relaxed);
    }

    void CTaskGraph_Impl::TryDecCounter()
    {
        uint32_t remainCounter = --m_PendingTaskCount;
        std::atomic_thread_fence(std::memory_order_acquire);
        if (remainCounter == 0)
        {
            Finalize();
            m_Promise.set_value();
            GetThreadManager().RemoveTaskGraph(this);
        }
    }

    void CTaskGraph_Impl::Finalize() const
    {
        if (m_Functor != nullptr)
        {
            m_Functor();
        }
    }


    void CThreadManager_Impl::ProcessingWorks(uint32_t threadId)
    {
        while (!m_Stopped)
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_ConditinalVariable.wait(lock, [this]()
            {
                return !m_TaskQueue.empty();
            });

            if (m_TaskQueue.empty())
            {
                lock.unlock();
                continue;
            }
            if (m_Stopped)
            {
                lock.unlock();
                return;
            }
            auto task = m_TaskQueue.front();
            m_TaskQueue.pop_front();
            lock.unlock();
            task->Invoke();
        }
    }

    CThreadManager_Impl::CThreadManager_Impl() : 
        m_TaskGraphPool([](CTaskGraph_Impl* graph) {}
        ,[](CTaskGraph_Impl* graph) {
        graph->ReleaseGraph();
        })
    {
    }

    void CThreadManager_Impl::InitializeThreadCount(uint32_t threadNum)
    {
        m_WorkerThreads.reserve(threadNum);
        for (uint32_t i = 0; i < threadNum; ++i)
        {
            m_WorkerThreads.emplace_back(&CThreadManager_Impl::ProcessingWorks, this, i);
        }
    }
    std::future<int> CThreadManager_Impl::EnqueueAnyThreadWorkWithPromise(std::function<void()> function)
    {
        return std::future<int>();
    }
    std::shared_future<void> CThreadManager_Impl::ExecuteTaskGraph(CTaskGraph* graph)
    {
        CTaskGraph_Impl* pGraph = static_cast<CTaskGraph_Impl*>(graph);
        pGraph->SetupTopology();
        if (pGraph->m_SourceTasks.empty())
        {
            RemoveTaskGraph(pGraph);
            pGraph->Finalize();
            auto dummy = std::promise<void>();
            dummy.set_value();
            return dummy.get_future();
        }
        std::atomic_thread_fence(std::memory_order_release);
        std::shared_future<void> graph_future(pGraph->m_Promise.get_future());
        EnqueueGraphTasks(pGraph->m_SourceTasks);
        return graph_future;
    }
    CTaskGraph* CThreadManager_Impl::NewTaskGraph()
    {
        CTaskGraph_Impl* newGraph = m_TaskGraphPool.Alloc(*this);
        return newGraph;
    }

    void CThreadManager_Impl::RemoveTaskGraph(CTaskGraph_Impl* graph)
    {
        m_TaskGraphPool.Release(graph);
    }

    void CThreadManager_Impl::EnqueueGraphTask(CTask_Impl* newTask)
    {
        {
            std::lock_guard<std::mutex> guard(m_Mutex);
            m_TaskQueue.push_back(newTask);
            m_ConditinalVariable.notify_one();
        }
    }

    void CThreadManager_Impl::EnqueueGraphTasks(std::vector<CTask_Impl*> const& tasks)
    {
        std::lock_guard<std::mutex> guard(m_Mutex);
        m_TaskQueue.insert(m_TaskQueue.end(), tasks.begin(), tasks.end());
        m_ConditinalVariable.notify_all();
    }*/

    //CA_LIBRARY_INSTANCE_LOADING_FUNCTIONS(CThreadManager, CThreadManager_Impl)

CTaskGraph* TaskGraph_Impl1::Name(std::string name)
{
    Name_Internal(name);
    return this;
}

CTaskGraph* TaskGraph_Impl1::DependsOn(CTask* parentTask)
{
    CTask_Impl1* task = static_cast<CTask_Impl1*>(parentTask);
    DependsOn_Internal(task);
    return this;
}

CTaskGraph* TaskGraph_Impl1::DependsOn(CTaskGraph* parentTask)
{
    TaskGraph_Impl1* task = static_cast<TaskGraph_Impl1*>(parentTask);
    DependsOn_Internal(task);
    return this;
}

TaskGraph_Impl1::TaskGraph_Impl1(TaskBaseObject* owner, ThreadManager_Impl1* owningManager) :
        TaskNode(TaskObjectType::eGraph, owner, owningManager)
    {
    }

    CTask_Impl1* TaskGraph_Impl1::NewTask_Internal()
    {
        std::lock_guard<std::mutex> guard(m_Mutex);
        m_TaskPool.emplace_back(this, m_OwningManager);
        CTask_Impl1* result = &m_TaskPool.back();
        m_SubTasks.push_back(result);
        return result;
    }

    TaskGraph_Impl1* TaskGraph_Impl1::NewSubTaskGraph_Internal()
    {
        std::lock_guard<std::mutex> guard(m_Mutex);
        m_TaskGraphPool.emplace_back(this, m_OwningManager);
        TaskGraph_Impl1* result = &m_TaskGraphPool.back();
        m_SubTasks.push_back(result);
        return result;
    }

    void TaskGraph_Impl1::NotifyChildNodeFinish(TaskNode* childNode)
    {
        uint32_t remainCounter = --m_PendingSubnodeCount;
        if (remainCounter == 0)
        {
            FinalizeExecution_Internal();
        }
    }
    void TaskGraph_Impl1::Execute_Internal()
    {
        SetupSubnodeDependencies();
        m_OwningManager->EnqueueTaskNodes(m_RootTasks);
    }

    void TaskGraph_Impl1::SetupSubnodeDependencies()
    {
        m_RootTasks.clear();
        for (TaskNode* itrTask : m_SubTasks)
        {
            itrTask->SetupThisNodeDependencies_Internal();
            if (itrTask->GetDepenedentCount() == 0)
            {
                m_RootTasks.push_back(itrTask);
            }
        }
        uint32_t pendingTaskCount = m_SubTasks.size();
        m_PendingSubnodeCount.store(pendingTaskCount, std::memory_order_release);
    }
    CTask* CTask_Impl1::Name(std::string name)
    {
        Name_Internal(name);
        return this;
    }
    CTask* CTask_Impl1::DependsOn(CTask* parentTask)
    {
        CTask_Impl1* task = static_cast<CTask_Impl1*>(parentTask);
        DependsOn_Internal(task);
        return this;
    }
    CTask* CTask_Impl1::DependsOn(CTaskGraph* parentTask)
    {
        TaskGraph_Impl1* task = static_cast<TaskGraph_Impl1*>(parentTask);
        DependsOn_Internal(task);
        return this;
    }
    std::shared_future<void> CTask_Impl1::Run()
    {
        StartExecute();
        return std::shared_future<void>();
    }
    CTask_Impl1::CTask_Impl1(TaskBaseObject* owner, ThreadManager_Impl1* owningManager) :
        TaskNode(TaskObjectType::eNode, owner, owningManager)
    {

    }

    CTask_Impl1& CTask_Impl1::Functor_Internal(std::function<void()> functor)
    {
        m_Functor = functor;
    }

    void CTask_Impl1::Release()
    {
        m_Functor = nullptr;
    }

    void CTask_Impl1::Execute_Internal()
    {
        if (m_Functor != nullptr)
        {
            m_Functor();
        }
        FinalizeExecution_Internal();
    }

    ThreadManager_Impl1::ThreadManager_Impl1() : 
        TaskBaseObject(TaskObjectType::eManager)
        , m_TaskGraphPool(threadsafe_utils::DefaultInitializer<TaskGraph_Impl1>{})
        , m_TaskPool(threadsafe_utils::DefaultInitializer<CTask_Impl1>{})
    {
    }

    void ThreadManager_Impl1::InitializeThreadCount(uint32_t threadNum)
    {
        m_WorkerThreads.reserve(threadNum);
        for (uint32_t i = 0; i < threadNum; ++i)
        {
            m_WorkerThreads.emplace_back(&ThreadManager_Impl1::ProcessingWorks, this, i);
        }
    }
    CTask_Impl1* ThreadManager_Impl1::NewTask()
    {
        return m_TaskPool.Alloc(this, this);
    }
    TaskGraph_Impl1* ThreadManager_Impl1::NewSubTaskGraph()
    {
        return m_TaskGraphPool.Alloc(this, this);
    }
    void ThreadManager_Impl1::EnqueueTaskNode(TaskNode* enqueueNode)
    {
        std::lock_guard<std::mutex> guard(m_Mutex);
        if (!enqueueNode->m_Running)
        {
            enqueueNode->m_Running.store(true, std::memory_order_relaxed);
            m_TaskQueue.push_back(enqueueNode);
            m_ConditinalVariable.notify_one();
        }
    }
    void ThreadManager_Impl1::EnqueueTaskNodes(std::deque<TaskNode*> const& nodeDeque)
    {
        std::lock_guard<std::mutex> guard(m_Mutex);
        uint32_t enqueuedCounter = 0;
        for (TaskNode* itrNode : nodeDeque)
        {
            if (!itrNode->m_Running)
            {
                itrNode->m_Running.store(true, std::memory_order_relaxed);
                m_TaskQueue.push_back(itrNode);
                ++enqueuedCounter;
            }
        };
        if (enqueuedCounter == 0)
            return;
        std::atomic_thread_fence(std::memory_order_release);
        if(enqueuedCounter > 1)
		{
			m_ConditinalVariable.notify_all();
		}
		else
		{
			m_ConditinalVariable.notify_one();
		}
    }
    void ThreadManager_Impl1::NotifyChildNodeFinish(TaskNode* childNode)
    {
        switch (childNode->GetTaskObjectType())
        {
            case TaskObjectType::eGraph:
			{
                m_TaskGraphPool.Release(static_cast<TaskGraph_Impl1*>(childNode));
				break;
			}
            case TaskObjectType::eNode:
            {
                m_TaskPool.Release(static_cast<CTask_Impl1*>(childNode));
                break;
            }
            default:
				CA_LOG_ERR("Invalid TaskNode Type");
				break;
        }
    }
    void ThreadManager_Impl1::ProcessingWorks(uint32_t threadId)
    {
        while (!m_Stopped)
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_ConditinalVariable.wait(lock, [this]()
                {
                    return !m_TaskQueue.empty();
                });

            if (m_TaskQueue.empty())
            {
                lock.unlock();
                continue;
            }
            if (m_Stopped)
            {
                lock.unlock();
                return;
            }
            auto task = m_TaskQueue.front();
            m_TaskQueue.pop_front();
            lock.unlock();
            task->Execute_Internal();
        }
    }
}


