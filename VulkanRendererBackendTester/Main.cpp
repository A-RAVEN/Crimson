#include <cstdlib>
#define NOMINMAX
#include <windows.h>
#include <RenderInterface/header/CRenderBackend.h>
#include <RenderInterface/header/CGPUPrimitiveResource.h>
#include <ThreadManager/header/ThreadManager.h>
#include <ShaderCompiler/header/Compiler.h>
#include <iostream>
#include <SharedTools/header/library_loader.h>
#include <RenderInterface/header/CNativeRenderPassInfo.h>
#include <RenderInterface/header/CCommandList.h>
#include <SharedTools/header/FileLoader.h>
#include "TestShaderProvider.h"
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

	auto shaderSource = fileloading_utils::LoadStringFile("D:/Projects/Crimson/Build/x64/Debug/testShader.hlsl");

	auto spirVResult = pShaderCompiler->CompileShaderSource(EShaderSourceType::eHLSL
		, "testShader.hlsl"
		, shaderSource
		, "vert"
		, ECompileShaderType::eVert);


	std::shared_ptr<TestShaderProvider> vertProvider = std::make_shared<TestShaderProvider>();
	vertProvider->SetUniqueName("testShader.hlsl.vert");
	vertProvider->SetData("spirv", "vert", spirVResult.data(), spirVResult.size() * sizeof(uint32_t));


	spirVResult = pShaderCompiler->CompileShaderSource(EShaderSourceType::eHLSL
		, "testShader.hlsl"
		, shaderSource
		, "frag"
		, ECompileShaderType::eFrag);

	std::shared_ptr<TestShaderProvider> fragProvider = std::make_shared<TestShaderProvider>();
	fragProvider->SetUniqueName("testShader.hlsl.frag");
	fragProvider->SetData("spirv", "frag", spirVResult.data(), spirVResult.size() * sizeof(uint32_t));

	GraphicsShaderSet shaderSet{};
	shaderSet.vert = vertProvider;
	shaderSet.frag = fragProvider;


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

	CVertexInputDescriptor vertexInputDesc{};
	vertexInputDesc.AddPrimitiveDescriptor(20, {
		VertexAttribute{0, offsetof(VertexData, x), VertexInputFormat::eR32G32_SFloat}
		, VertexAttribute{1, offsetof(VertexData, r), VertexInputFormat::eR32G32B32_SFloat}
		});


	CAttachmentInfo attachmentInfo{};
	attachmentInfo.format = ETextureFormat::E_R8G8B8A8_UNORM;
	attachmentInfo.loadOp = EAttachmentLoadOp::eClear;
	attachmentInfo.clearValue = GraphicsClearValue::ClearColor(0.0f, 1.0f, 1.0f, 1.0f);
	CRenderpassBuilder newRenderPass{ {
		attachmentInfo
	} };

	newRenderPass.Subpass({ {0} }, CPipelineStateObject{}, vertexInputDesc, shaderSet, [vertexBuffer, indexBuffer](CInlineCommandList& cmd)
		{
			if (vertexBuffer->UploadingDone() && indexBuffer->UploadingDone())
			{
				cmd.BindVertexBuffers({ vertexBuffer.get() }, {});
				cmd.BindIndexBuffers(EIndexBufferType::e16, indexBuffer.get());
				cmd.DrawIndexed(3);
			}
			else
			{
				std::cout << "Not Finish Yet" << std::endl;
			}
		});

	while (pBackend->AnyWindowRunning())
	{
		pBackend->StartCurrentFrame();

		if (frame == 0)
		{
			vertexBuffer->DoUpload();
			indexBuffer->DoUpload();
		}

		pBackend->ExecuteRenderPass(newRenderPass);
		pBackend->EndCurrentFrame();
		pBackend->TickWindows();
		++frame;
		std::this_thread::sleep_for(std::chrono::microseconds(10));
	}
	pBackend->Release();
	pBackend.reset();

	pThreadManager.reset();

	return EXIT_SUCCESS;
}
