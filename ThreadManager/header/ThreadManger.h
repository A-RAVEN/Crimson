#pragma once
#include <functional>
#include <future>

namespace thread_management
{
	class CThreadManager
	{
	public:
		virtual void InitializeThreadCount(uint32_t threadNum) = 0;
		virtual std::future<int> EnqueueAnyThreadWorkWithPromise(std::function<void()> function) = 0;
	};
}