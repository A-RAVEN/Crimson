#include "pch.h"
#include "ThreadManager_Impl.h"

namespace thread_management
{
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

    std::shared_future<void> TaskGraph_Impl1::Run()
    {
        return StartExecute();
    }

    CTask* TaskGraph_Impl1::NewTask()
    {
        return NewTask_Internal();
    }

    CTaskGraph* TaskGraph_Impl1::NewTaskGraph()
    {
        return NewSubTaskGraph_Internal();
    }

    TaskGraph_Impl1::TaskGraph_Impl1(TaskBaseObject* owner, ThreadManager_Impl1* owningManager) :
        TaskNode(TaskObjectType::eGraph, owner, owningManager)
    {
    }

    void TaskGraph_Impl1::Release()
    {
        m_PendingSubnodeCount.store(0, std::memory_order_relaxed);
        m_RootTasks.clear();
        m_SubTasks.clear();
        for (auto& ref_subtask : m_TaskPool)
        {
            ref_subtask.Release();
        }
        m_TaskPool.clear();
        for (auto p_subtaskgraph : m_TaskGraphPool)
        {
            p_subtaskgraph->Release();
            delete p_subtaskgraph;
        }
        m_TaskGraphPool.clear();
        Release_Internal();
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
        m_TaskGraphPool.emplace_back(new TaskGraph_Impl1{ this, m_OwningManager });
        TaskGraph_Impl1* result = m_TaskGraphPool.back();
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
        if (m_SubTasks.empty())
        {
            FinalizeExecution_Internal();
            return;
        }
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
        return StartExecute();
    }
    CTask* CTask_Impl1::Functor(std::function<void()> functor)
    {
        Functor_Internal(functor);
        return this;
    }
    CTask_Impl1::CTask_Impl1(TaskBaseObject* owner, ThreadManager_Impl1* owningManager) :
        TaskNode(TaskObjectType::eNode, owner, owningManager)
    {
    }

    void CTask_Impl1::Functor_Internal(std::function<void()> functor)
    {
        m_Functor = functor;
    }

    void CTask_Impl1::Release()
    {
        m_Functor = nullptr;
        Release_Internal();
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

    ThreadManager_Impl1::~ThreadManager_Impl1()
    {
        {
            m_Stopped = true;
            m_ConditinalVariable.notify_all();
        }
        for (std::thread& itrThread : m_WorkerThreads)
        {
			itrThread.join();
		}
    }

    void ThreadManager_Impl1::InitializeThreadCount(uint32_t threadNum)
    {
        m_WorkerThreads.reserve(threadNum);
        for (uint32_t i = 0; i < threadNum; ++i)
        {
            m_WorkerThreads.emplace_back(&ThreadManager_Impl1::ProcessingWorks, this, i);
        }
    }
    CTask* ThreadManager_Impl1::NewTask()
    {
        return m_TaskPool.Alloc(this, this);
    }
    CTaskGraph* ThreadManager_Impl1::NewTaskGraph()
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
                    //TaskQueue不是空的，或者线程管理器已经停止，不再等待
                    return m_Stopped || !m_TaskQueue.empty();
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
            //std::cout << task->m_Name << std::endl;
            task->Execute_Internal();
        }
    }

    CA_LIBRARY_INSTANCE_LOADING_FUNCTIONS(CThreadManager, ThreadManager_Impl1)
}


