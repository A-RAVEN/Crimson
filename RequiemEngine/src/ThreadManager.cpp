#include <headers/ThreadManager.h>
#include <algorithm>
#include <iostream>

void ThreadWorker::Work()
{
	while (p_OwningManager->b_Working)
	{
		ThreadJob* new_job = p_OwningManager->DequeueJob();
		if (new_job)
		{
			new_job->Work(this);
		}
		new_job->b_Finished = true;
	}
}

void ThreadWorker::End()
{
	m_WorkerThread.join();
}

bool ThreadWorker::Working() const
{
	return p_OwningManager->b_Working;
}

uint32_t ThreadWorker::GetId() const
{
	return m_ThreadId;
}

ThreadManager::ThreadManager() :
	b_Working(false)
{
}

void ThreadManager::Init()
{
	b_Working = true;
	uint32_t hardware_thread = std::thread::hardware_concurrency();
	m_Workers.resize(((std::max)(1u, hardware_thread - 2)));
	uint32_t id = 0;
	for (auto& worker : m_Workers)
	{
		worker = new ThreadWorker(this, id++);
	}
}

void ThreadManager::Terminate()
{
	b_Working = false;
	m_QueueNotifier.notify_all();
	for (auto worker : m_Workers)
	{
		worker->End();
		delete worker;
	}
	m_Workers.clear();
}

void ThreadManager::EnqueueJob(ThreadJob* job)
{
	if (!job->b_Finished)
	{
		std::cout << "job not finish yet" << std::endl;
		return;
	}
	job->b_Finished = false;
	LockGuard guard(m_QueueMutex);
	m_JobQueue.push_back(job);
	m_QueueNotifier.notify_one();
}

ThreadJob* ThreadManager::DequeueJob()
{
	UniqueLock lock(m_QueueMutex);
	while (m_JobQueue.empty())
	{
		if (!b_Working){return nullptr;}
		m_QueueNotifier.wait(lock);
	}
	if (!b_Working) { return nullptr; }
	ThreadJob* front = m_JobQueue.front();
	m_JobQueue.pop_front();
	return front;
}

void ThreadJob::WaitJob()
{
	while (!b_Finished){}
}
