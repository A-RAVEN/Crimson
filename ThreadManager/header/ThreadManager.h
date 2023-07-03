#pragma once
#include <functional>
#include <future>

namespace thread_management
{
#ifdef THREADMANAGER_EXPORTS
#define THREADMANAGER_API __declspec(dllexport)
#else
#define THREADMANAGER_API
#endif

	class CTask
	{
	public:
		virtual ~CTask() = default;
		CTask() = default;
		CTask(CTask const& other) = delete;
		CTask& operator=(CTask const& other) = delete;
		CTask(CTask&& other) = default;
		CTask& operator=(CTask&& other) = default;
		virtual CTask* Succeed(CTask* parentTask) = 0;
		virtual CTask* Name(std::string name) = 0;
		virtual CTask* Functor(std::function<void()> functor) = 0;
	};

	class CTaskGraph
	{
	public:
		virtual ~CTaskGraph() = default;
		CTaskGraph() = default;
		CTaskGraph(CTaskGraph const& other) = delete;
		CTaskGraph& operator=(CTaskGraph const& other) = delete;
		CTaskGraph(CTaskGraph && other) = default;
		CTaskGraph& operator=(CTaskGraph && other) = default;
		virtual CTaskGraph* Name(std::string name) = 0;
		virtual CTask* NewTask() = 0;
		virtual CTaskGraph* FinalizeFunctor(std::function<void()> functor) = 0;
	};

	class CThreadManager
	{
	public:
		virtual ~CThreadManager() = default;
		CThreadManager() = default;
		CThreadManager(CThreadManager const& other) = delete;
		CThreadManager& operator=(CThreadManager const& other) = delete;
		CThreadManager(CThreadManager&& other) = default;
		CThreadManager& operator=(CThreadManager&& other) = default;
		virtual void InitializeThreadCount(uint32_t threadNum) = 0;
		virtual std::future<int> EnqueueAnyThreadWorkWithPromise(std::function<void()> function) = 0;
		virtual std::future<void> ExecuteTaskGraph(CTaskGraph* graph) = 0;
		virtual CTaskGraph* NewTaskGraph() = 0;
	};

	extern "C"
	{
		THREADMANAGER_API CThreadManager* NewModuleInstance();
		THREADMANAGER_API void DeleteModuleInstance(CThreadManager* removingManager);
	}

#define GETNAME(VAR) #VAR

	typedef CThreadManager* (*FTP_NewThreadManager)();
	typedef void(*FPT_DeleteThreadManager)(CThreadManager*);

#ifndef THREADMANAGER_EXPORTS
	void LoadModuleLibrary();
	void UnloadModuleLibrary();
#endif

#if defined(DEFINE_THREADMANAGER_LOAD_FUNCTIONS) && !defined(THREADMANAGER_EXPORTS)

	HINSTANCE hModuleLib = nullptr;
	FTP_NewThreadManager pNewInstanceFunc = nullptr;
	FPT_DeleteThreadManager pDeleteInstanceFunc = nullptr;

	void LoadModuleLibrary()
	{
		hModuleLib = LoadLibrary(L"ThreadManager");
		if (hModuleLib != nullptr)
		{
			pNewInstanceFunc = reinterpret_cast<FTP_NewThreadManager>(GetProcAddress(hModuleLib, GETNAME(NewModuleInstance)));
			pDeleteInstanceFunc = reinterpret_cast<FPT_DeleteThreadManager>(GetProcAddress(hModuleLib, GETNAME(DeleteModuleInstance)));
		}
	}

	void UnloadModuleLibrary()
	{
		pNewInstanceFunc = nullptr;
		pDeleteInstanceFunc = nullptr;
		if (hModuleLib != nullptr)
		{
			FreeLibrary(hModuleLib);
			hModuleLib = nullptr;
		}
	}

	CThreadManager* NewModuleInstance()
	{
		return pNewInstanceFunc();
	}

	void DeleteModuleInstance(CThreadManager* removingBackend)
	{
		pDeleteInstanceFunc(removingBackend);
	}
#endif
}