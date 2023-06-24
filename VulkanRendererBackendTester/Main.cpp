#include <cstdlib>
#define NOMINMAX
#include <windows.h>
#define DEFINE_RENDERBACKEND_LOAD_FUNCTIONS
#include <CRenderBackend.h>
#include "ThreadManager.h"
#include <iostream>


int main(int argc, char *argv[])
{
	ThreadManager threadMgr(5);

	graphics_backend::LoadRenderBackend();

	graphics_backend::CRenderBackend* pBackend = graphics_backend::NewRenderBackend();
	pBackend->Initialize("Test Vulkan Backend", "CRIMSON Engine");
	pBackend->InitializeThreadContextCount(1);
	pBackend->NewWindow(1024, 512, "Test Window");
	while (pBackend->AnyWindowRunning())
	{
		pBackend->TickTest();
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
	threadMgr.StopTasks();
	return EXIT_SUCCESS;
}
