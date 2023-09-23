#pragma once
#include <future>
#include <vector>
namespace thread_management
{
	class ThreadManager_Impl1;
	class TaskNode;
	enum class TaskObjectType
	{
		eManager = 0,
		eGraph,
		eNode,
		eNodeParallel,
	};

	class TaskBaseObject
	{
	public:
		TaskBaseObject(TaskObjectType type) :m_Type(type) {}
		virtual void NotifyChildNodeFinish(TaskNode* childNode) {}
	//protected:
		TaskObjectType GetTaskObjectType() const { return m_Type; }
	private:
		TaskObjectType m_Type;
	};

	class TaskNode : public TaskBaseObject
	{
	public:
		TaskNode(TaskObjectType type, TaskBaseObject* owner, ThreadManager_Impl1* owningManager);
		std::shared_future<void> StartExecute();
		virtual void NotifyChildNodeFinish(TaskNode* childNode) override {}
		virtual void Execute_Internal() = 0;
		virtual void SetupSubnodeDependencies() {};
		void SetupThisNodeDependencies_Internal();
		size_t GetDepenedentCount() const { return m_Dependents.size(); }
		void Release_Internal();
		std::shared_future<void> AquireFuture();
		void FulfillPromise();
	protected:
		void NotifyDependsOnFinish(TaskNode* dependsOnNode);
		void Name_Internal(const std::string& name);
		void DependsOn_Internal(TaskNode* dependsOnNode);
		void FinalizeExecution_Internal();
	protected:
		ThreadManager_Impl1* m_OwningManager;
		TaskBaseObject* m_Owner;
		std::atomic_bool m_Running{ false };
		std::string m_Name;
		std::vector<TaskNode*>m_Dependents;
		std::vector<TaskNode*>m_Successors;
		std::atomic<uint32_t>m_PendingDependsOnTaskCount{0};

		bool m_HasPromise = false;
		std::promise<void> m_Promise;

		friend class ThreadManager_Impl1;
	};
}


