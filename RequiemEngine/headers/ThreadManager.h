#pragma once
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include <list>
#include <headers/Containers/ConcurrentQueue/concurrentqueue.h>

using LockGuard = std::lock_guard<std::mutex>;
using UniqueLock = std::unique_lock<std::mutex>;

constexpr uint32_t STATIC_JOB_QUEUE_CAPACITY = 100;

class ThreadManager;
class ThreadWorker;

class ThreadJob
{
public:
	ThreadJob() : b_Finished(true) {}
	virtual ~ThreadJob() {};
	virtual void Work(ThreadWorker const* this_worker) {};
	void WaitJob();
protected:
	friend class ThreadManager;
	friend class ThreadWorker;
	std::atomic_bool b_Finished;
};

class ThreadWorker
{
public:
	ThreadWorker(ThreadManager* manager, uint32_t id) :
		p_OwningManager(manager),
		m_ThreadId(id),
		m_WorkerThread(&ThreadWorker::Work, this)
	{
	}
	void Work();
	void End();
	bool Working() const;
	uint32_t GetId() const;
	friend class ThreadManager;
private:
	ThreadManager* p_OwningManager;
	uint32_t m_ThreadId;
	std::thread m_WorkerThread;
	moodycamel::ProducerToken* producerToken;
	moodycamel::ConsumerToken* consumerToken;
};

class ThreadManager
{
public:
	ThreadManager();
	void Init();
	void Terminate();
	void EnqueueJob(ThreadJob* job);
	void EnqueueJob(ThreadWorker const* srcWorker, ThreadJob* job);
	ThreadJob* DequeueJob();
	friend class ThreadWorker;
private:
	std::vector<ThreadWorker*> m_Workers;
	std::mutex m_QueueMutex;
	std::condition_variable m_QueueNotifier;
	moodycamel::ConcurrentQueue<ThreadJob*> m_JobQueue;
	//std::list<ThreadJob*> m_JobQueue;
	std::atomic_bool b_Working;
};