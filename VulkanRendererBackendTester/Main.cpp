#include <cstdlib>
#define NOMINMAX
#include <windows.h>
#include <RenderInterface/header/CRenderBackend.h>
#include <RenderInterface/header/CGPUPrimitiveResource.h>
#include <ThreadManager/header/ThreadManager.h>
#include <ShaderCompiler/header/Compiler.h>
#include <iostream>
#include <SharedTools/header/library_loader.h>
using namespace thread_management;
using namespace library_loader;
using namespace graphics_backend;
using namespace ShaderCompiler;

int main(int argc, char *argv[])
{
	TModuleLoader<CThreadManager> threadManagerLoader(L"ThreadManager");
	TModuleLoader<CRenderBackend> renderBackendLoader(L"VulkanRenderBackend");
	TModuleLoader<IShaderCompiler> shaderCompilerLoader(L"ShaderCompiler");

	auto pThreadManager = threadManagerLoader.New();
	pThreadManager->InitializeThreadCount(5);

	auto pShaderCompiler = shaderCompilerLoader.New();

	auto pBackend = renderBackendLoader.New();
	pBackend->Initialize("Test Vulkan Backend", "CRIMSON Engine");
	pBackend->InitializeThreadContextCount(pThreadManager.get(), 1);
	pBackend->NewWindow(1024, 512, "Test Window");

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
		pBackend->TestCode();
		pBackend->EndCurrentFrame();
		pBackend->TickWindows();
		++frame;
		std::this_thread::sleep_for(std::chrono::microseconds(10));
	}
	pBackend->ReleaseGPUPrimitiveResource(pPrimitive);
	pBackend->Release();
	pBackend.reset();

	pThreadManager.reset();

	return EXIT_SUCCESS;
}
