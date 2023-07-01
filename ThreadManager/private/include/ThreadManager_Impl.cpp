#include "private/include/pch.h"
#include "ThreadManager_Impl.h"

namespace thread_management
{
    CTask_Impl::CTask_Impl(CTaskGraph_Impl& owningGraph) : m_OwningGraph(owningGraph)
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
        m_PendingTaskCount.store(m_PendingTaskCount, std::memory_order_relaxed);
    }
    void CTask_Impl::Invoke()
    {
        m_Functor();
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
            m_OwningGraph.GetThreadMaanger().EnqueueGraphTask(this);
        }
    }
    CTaskGraph_Impl::CTaskGraph_Impl(CThreadManager_Impl& owningManager) : m_OwningManager(owningManager)
    {
    }
    CTask* CTaskGraph_Impl::NewTask()
    {
        m_Tasks.emplace_back(*this);
        return &m_Tasks.back();
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
    }

    void CTaskGraph_Impl::TryDecCounter()
    {
        uint32_t remainCounter = --m_PendingTaskCount;
        std::atomic_thread_fence(std::memory_order_acquire);
        if (remainCounter == 0)
        {

        }
    }


    void CThreadManager_Impl::ProcessingWorks()
    {
        while (!m_Stopped)
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_ConditinalVariable.wait(lock);
            if (m_Stopped)
                return;
            if (m_TaskQueue.empty())
            {
                lock.unlock();
                continue;
            }
            auto task = m_TaskQueue.front();
            m_TaskQueue.pop_front();
            lock.unlock();
            task->Invoke();
        }
    }

    void CThreadManager_Impl::InitializeThreadCount(uint32_t threadNum)
    {
        m_WorkerThreads.reserve(threadNum);
        for (size_t i = 0; i < threadNum; ++i)
        {
            m_WorkerThreads.emplace_back(&CThreadManager_Impl::ProcessingWorks, this);
        }
    }
    std::future<int> CThreadManager_Impl::EnqueueAnyThreadWorkWithPromise(std::function<void()> function)
    {
        return std::future<int>();
    }
    void CThreadManager_Impl::ExecuteTaskGraph(CTaskGraph* graph)
    {
    }
    CTaskGraph* CThreadManager_Impl::NewTaskGraph()
    {
        m_TaskGraphList.emplace_back(*this);
        CTaskGraph* newGraph = &m_TaskGraphList.back();
        return newGraph;
    }
    void CThreadManager_Impl::EnqueueGraphTask(CTask_Impl* newTask)
    {
        std::unique_lock<std::mutex> lock(m_Mutex);
        m_TaskQueue.push_back(newTask);
        m_ConditinalVariable.notify_one();
    }
}


