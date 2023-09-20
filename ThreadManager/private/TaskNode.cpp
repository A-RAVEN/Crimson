#include "pch.h"
#include "TaskNode.h"
#include <SharedTools/header/DebugUtils.h>
#include "ThreadManager_Impl.h"

namespace thread_management
{
	TaskNode::TaskNode(TaskObjectType type, TaskBaseObject* owner, ThreadManager_Impl1* owningManager) : TaskBaseObject(type)
		, m_Owner(owner)
		, m_OwningManager(owningManager)
	{
	}
	std::shared_future<void> TaskNode::StartExecute()
	{
		if (m_Owner->GetTaskObjectType() == TaskObjectType::eManager
			&& m_Owner == m_OwningManager
			&& m_Dependents.empty()
			&& !m_Running)
		{
			SetupThisNodeDependencies_Internal();
			m_OwningManager->EnqueueTaskNode(this);
			return AquireFuture();
		}
		return std::shared_future<void>();
	}
	void TaskNode::NotifyDependsOnFinish(TaskNode* dependsOnNode)
	{
		uint32_t remainCounter = --m_PendingDependsOnTaskCount;
		if (remainCounter == 0)
		{
			m_OwningManager->EnqueueTaskNode(this);
		}
	}
	void TaskNode::Name_Internal(const std::string& name)
	{
		m_Name = name;
	}
	void TaskNode::DependsOn_Internal(TaskNode* dependsOnNode)
	{
		CA_ASSERT(dependsOnNode->m_Owner == m_Owner, "Dependency Is Valid For Nodes Under Same Owner");
		dependsOnNode->m_Successors.push_back(this);
		m_Dependents.push_back(dependsOnNode);
	}
	void TaskNode::SetupThisNodeDependencies_Internal()
	{
		uint32_t pendingCount = m_Dependents.size();
		m_PendingDependsOnTaskCount.store(pendingCount, std::memory_order_relaxed);
	}
	void TaskNode::Release_Internal()
	{
		FulfillPromise();
		m_Running.store(false, std::memory_order_relaxed);
		m_PendingDependsOnTaskCount.store(0, std::memory_order_relaxed);
		m_Name = "";
		m_Dependents.clear();
		m_Successors.clear();
		m_HasPromise = false;
	}
	std::shared_future<void> TaskNode::AquireFuture()
	{
		CA_ASSERT(!m_HasPromise, "Future Already Aquired");
		m_HasPromise = true;
		m_Promise = std::promise<void>();
		std::shared_future<void> nodeFuture(m_Promise.get_future());
		return nodeFuture;
	}
	void TaskNode::FulfillPromise()
	{
		if (m_HasPromise)
		{
			m_HasPromise = false;
			m_Promise.set_value();
		}
	}
	void TaskNode::FinalizeExecution_Internal()
	{
		std::atomic_thread_fence(std::memory_order_release);
		for (auto itrSuccessor = m_Successors.begin(); itrSuccessor != m_Successors.end(); ++itrSuccessor)
		{
			(*itrSuccessor)->NotifyDependsOnFinish(this);
		}
		m_Owner->NotifyChildNodeFinish(this);
	}
}

