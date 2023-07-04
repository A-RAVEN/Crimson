#include <cstdlib>
#define NOMINMAX
#include <windows.h>
#define DEFINE_RENDERBACKEND_LOAD_FUNCTIONS
#include <RenderInterface/header/CRenderBackend.h>
#include <RenderInterface/header/CGPUPrimitiveResource.h>
#define DEFINE_THREADMANAGER_LOAD_FUNCTIONS
#include <ThreadManager/header/ThreadManager.h>
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
	//{
	//	int32_t thisFrame = frame;
	//	++frame;
	//	auto newGraph = pThreadManager->NewTaskGraph();
	//	newGraph->Name("Graph0")
	//		->FinalizeFunctor([]() {
	//		std::cout << "Finalize!!!" << std::endl;
	//		})
	//		;
	//	auto task0 = newGraph->NewTask();
	//	task0->Name("task0")
	//		->Functor([thisFrame]() {
	//		std::string result = "frame" + std::to_string(thisFrame) + " task0";
	//		std::cout << result << std::endl;
	//			});

	//	auto task1 = newGraph->NewTask();
	//	task1->Name("task1")
	//		->Functor([thisFrame]() {
	//		std::string result = "frame" + std::to_string(thisFrame) + " task1";
	//		std::cout << result << std::endl;
	//			});

	//	auto task2 = newGraph->NewTask();
	//	task2->Name("task2")
	//		->Functor([thisFrame]() {
	//		std::string result = "frame" + std::to_string(thisFrame) + " task2";
	//		std::cout << result << std::endl;
	//			});

	//	task0->Succeed(task1);
	//	task0->Succeed(task2);
	//	task2->Succeed(task1);

	//	pThreadManager->ExecuteTaskGraph(newGraph);
	//}

	int32_t frame = 0;

	graphics_backend::CGPUPrimitiveResource* pPrimitive = nullptr;

	while (pBackend->AnyWindowRunning())
	{
		pBackend->StartCurrentFrame();
		if(frame == 0)
		{
			pPrimitive = pBackend->NewGPUPrimitiveResource();
			pPrimitive->AddPrimitiveDescriptor(sizeof(int), { {0, 0, sizeof(int)} });
			std::vector<int> tmpData = { 1, 2, 3, 4, 5 };
			pPrimitive->SetPrimitiveData(0, tmpData.size() * sizeof(tmpData[0]), tmpData.data());
			pPrimitive->Submit();
		}
		if(pPrimitive->GPUDone())
		{
			//std::cout << "Done" << std::endl;
		}
		pBackend->EndCurrentFrame();
		pBackend->TickWindows();
		++frame;
	}
	pBackend->ReleaseGPUPrimitiveResource(pPrimitive);
	pBackend->Release();
	graphics_backend::DeleteRenderBackend(pBackend);

	graphics_backend::UnloadRenderBackend();
	thread_management::UnloadModuleLibrary();
	return EXIT_SUCCESS;
}
