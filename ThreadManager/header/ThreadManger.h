#pragma once
#include <functional>
#include <future>

namespace thread_management
{
	class CTask
	{
	public:
		virtual CTask* Succeed(CTask const* CTask) = 0;
		virtual CTask* Name(std::string name) = 0;
		virtual CTask* Functor(std::function<void()> functor) = 0;
	};

	class CTaskGraph
	{
	public:
		virtual CTask* NewTask() = 0;

	};

	class CThreadManager
	{
	public:
		virtual void InitializeThreadCount(uint32_t threadNum) = 0;
		virtual std::future<int> EnqueueAnyThreadWorkWithPromise(std::function<void()> function) = 0;
		virtual void ExecuteTaskGraph(CTaskGraph* )
	};
}