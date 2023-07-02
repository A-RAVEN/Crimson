#include <cstdlib>
#define NOMINMAX
#include <windows.h>
#define DEFINE_RENDERBACKEND_LOAD_FUNCTIONS
#include <RenderInterface/header/CRenderBackend.h>
#define DEFINE_THREADMANAGER_LOAD_FUNCTIONS
#include <ThreadManager/header/ThreadManager.h>
#include "ThreadManager.h"
#include <iostream>
using namespace thread_management;

int main(int argc, char *argv[])
{
	thread_management::LoadModuleLibrary();

	CThreadManager* pThreadManager = thread_management::NewModuleInstance();
	pThreadManager->InitializeThreadCount(5);

	graphics_backend::LoadRenderBackend();

	graphics_backend::CRenderBackend* pBackend = graphics_backend::NewRenderBackend();
	pBackend->Initialize("Test Vulkan Backend", "CRIMSON Engine");
	pBackend->InitializeThreadContextCount(pThreadManager, 1);
	pBackend->NewWindow(1024, 512, "Test Window");
	int32_t frame = 0;
	{
		int32_t thisFrame = frame;
		++frame;
		auto newGraph = pThreadManager->NewTaskGraph();
		newGraph->Name("Graph0")
			->FinalizeFunctor([]() {
			std::cout << "Finalize!!!" << std::endl;
			})
			;
		auto task0 = newGraph->NewTask();
		task0->Name("task0")
			->Functor([thisFrame]() {
			std::string result = "frame" + std::to_string(thisFrame) + " task0";
			std::cout << result << std::endl;
				});

		auto task1 = newGraph->NewTask();
		task1->Name("task1")
			->Functor([thisFrame]() {
			std::string result = "frame" + std::to_string(thisFrame) + " task1";
			std::cout << result << std::endl;
				});

		auto task2 = newGraph->NewTask();
		task2->Name("task2")
			->Functor([thisFrame]() {
			std::string result = "frame" + std::to_string(thisFrame) + " task2";
			std::cout << result << std::endl;
				});

		task0->Succeed(task1);
		task0->Succeed(task2);
		task2->Succeed(task1);

		pThreadManager->ExecuteTaskGraph(newGraph);
	}
	while (pBackend->AnyWindowRunning())
	{


		//pBackend->TickTest();
		//for (int i = 0; i < 3; ++i)
		//{
		//	threadMgr.EnqueueTask([]()
		//		{
		//			std::cout << std::this_thread::get_id() << std::endl;
		//		});
		//}
		pBackend->TickWindows();
	}
	pBackend->Release();
	graphics_backend::DeleteRenderBackend(pBackend);

	graphics_backend::UnloadRenderBackend();
	thread_management::UnloadModuleLibrary();
	return EXIT_SUCCESS;
}
