#pragma once
#include <functional>
#include <future>

namespace thread_management
{
	class CTask
	{
	public:
		virtual CTask* Succeed(CTask const* CTask);
		virtual CTask* Name(std::string name);
	};

	class CTaskGraph
	{
	public:

	};

	class CThreadManager
	{
	public:
		virtual void InitializeThreadCount(uint32_t threadNum) = 0;
		virtual std::future<int> EnqueueAnyThreadWorkWithPromise(std::function<void()> function) = 0;
	};
}