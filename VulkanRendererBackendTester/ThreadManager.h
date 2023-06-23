#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <functional>

class CThreadTaskBase
{
public:
	void Process();
};

class ThreadManager
{
public:
	ThreadManager(size_t workerCount);
	void EnqueueTask(std::function<void()> func);
	void StopTasks();
private:
	void ProcessingWorks();
	std::queue<std::function<void()>> m_TaskQueue;
	std::vector<std::thread> m_WorkerThreads;

	bool m_Stopped = false;

	std::mutex m_Mutex;
	std::condition_variable m_ConditinalVariable;
};

