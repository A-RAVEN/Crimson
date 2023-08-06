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
using namespace uenum;

struct VertexData
{
	float x;
	float y;
	float r;
	float g;
	float b;
};

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

	std::vector<VertexData> vertexDataList = {
		VertexData{-0.4f, -0.4f, 1.0f, 0.0f, 0.0f},
		VertexData{0.4f, -0.4f, 1.0f, 0.0f, 0.0f},
		VertexData{0.0f, 0.4f, 1.0f, 0.0f, 0.0f},
	};

	std::vector<uint16_t> indexDataList = {
		0, 1, 2
	};

	auto vertexBuffer = pBackend->CreateGPUBuffer(
		EBufferUsage::eVertexBuffer | EBufferUsage::eDataDst, 3, sizeof(VertexData));
	vertexBuffer->ScheduleBufferData(0, vertexDataList.size() * sizeof(VertexData), vertexDataList.data());

	auto indexBuffer = pBackend->CreateGPUBuffer(
		EBufferUsage::eIndexBuffer | EBufferUsage::eDataDst, 3, sizeof(uint16_t));
	indexBuffer->ScheduleBufferData(0, indexDataList.size() * sizeof(uint16_t), indexDataList.data());

	int32_t frame = 0;

	CGPUPrimitiveResource* pPrimitive = nullptr;

	pBackend->StartCurrentFrame();
	vertexBuffer->DoUpload();
	indexBuffer->DoUpload();
	//pPrimitive = pBackend->NewGPUPrimitiveResource();
	//pPrimitive->m_VertexInputDescriptor.AddPrimitiveDescriptor(sizeof(int), { {0, 0, VertexInputFormat::eR32_SInt} });
	//std::vector<int> tmpData = { 1, 2, 3, 4, 5 };
	//pPrimitive->SetPrimitiveData(0, tmpData.size() * sizeof(tmpData[0]), tmpData.data());
	//pPrimitive->Submit();
	pBackend->EndCurrentFrame();


	while (pBackend->AnyWindowRunning())
	{
		pBackend->StartCurrentFrame();
		//pBackend->TestCode();
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
