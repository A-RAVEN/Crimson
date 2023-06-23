#include "ThreadManager.h"

ThreadManager::ThreadManager(size_t workerCount)
{
	m_WorkerThreads.reserve(workerCount);
	for (size_t i = 0; i < workerCount; ++i)
	{
		m_WorkerThreads.emplace_back(&ThreadManager::ProcessingWorks, this);
	}
}

void ThreadManager::EnqueueTask(std::function<void()> func)
{
	std::unique_lock<std::mutex> lock(m_Mutex);
	m_TaskQueue.push(func);
	m_ConditinalVariable.notify_one();
}

void ThreadManager::StopTasks()
{
	{
		std::unique_lock<std::mutex> lock(m_Mutex);
		m_Stopped = true;
		m_ConditinalVariable.notify_all();
	}
	for (size_t i = 0; i < m_WorkerThreads.size(); ++i)
	{
		m_WorkerThreads[i].join();
	}
}

void ThreadManager::ProcessingWorks()
{
	while(!m_Stopped)
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
		m_TaskQueue.pop();
		lock.unlock();

		task();
	}
}
