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

	auto shaderModule = pBackend->NewShaderModule_Ptr();

	int32_t frame = 0;

	CGPUPrimitiveResource* pPrimitive = nullptr;

	pBackend->StartCurrentFrame();
	pPrimitive = pBackend->NewGPUPrimitiveResource();
	pPrimitive->m_VertexInputDescriptor.AddPrimitiveDescriptor(sizeof(int), { {0, 0, VertexInputFormat::eR32_SInt} });
	std::vector<int> tmpData = { 1, 2, 3, 4, 5 };
	pPrimitive->SetPrimitiveData(0, tmpData.size() * sizeof(tmpData[0]), tmpData.data());
	pPrimitive->Submit();
	pBackend->EndCurrentFrame();


	while (pBackend->AnyWindowRunning())
	{
		pBackend->StartCurrentFrame();

		pBackend->EndCurrentFrame();
		pBackend->TickWindows();
		++frame;
		std::this_thread::sleep_for(std::chrono::microseconds(10));
	}
	shaderModule = nullptr;
	pBackend->ReleaseGPUPrimitiveResource(pPrimitive);
	pBackend->Release();
	graphics_backend::DeleteRenderBackend(pBackend);

	graphics_backend::UnloadRenderBackend();
	thread_management::UnloadModuleLibrary();
	return EXIT_SUCCESS;
}
