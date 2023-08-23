#include <cstdlib>
#define NOMINMAX
#include <windows.h>
#include <RenderInterface/header/CRenderBackend.h>
#include <RenderInterface/header/CGPUPrimitiveResource.h>
#include <ThreadManager/header/ThreadManager.h>
#include <ShaderCompiler/header/Compiler.h>
#include <iostream>
#include <SharedTools/header/library_loader.h>
#include <RenderInterface/header/RenderInterfaceManager.h>
#include <RenderInterface/header/CNativeRenderPassInfo.h>
#include <RenderInterface/header/CCommandList.h>
#include <SharedTools/header/FileLoader.h>
#include "TestShaderProvider.h"
#include <RenderInterface/header/ShaderBindingBuilder.h>
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
	TModuleLoader<RenderInterfaceManager> renderInterfaceLoader(L"RenderInterface");

	ShaderBindingBuilder shaderBindingBuilder{ "TestBinding" };
	shaderBindingBuilder.Scalar<float>("test0")
		.Scalar<int>("test1")
		.Scalar<uint32_t, 2>("test2")
		.Vec2<float>("test4")
		.Vec3<int, 3>("test5")
		.Vec4<uint32_t, 2>("test6")
		.Mat2<float>("test7")
		.Mat3<int, 3>("test8")
		.Mat4<uint32_t, 2>("test9");

	auto pThreadManager = threadManagerLoader.New();
	pThreadManager->InitializeThreadCount(5);

	auto pRenderInterface = renderInterfaceLoader.New();

	auto pRenderGraph = pRenderInterface->NewRenderGraph();

	auto pShaderCompiler = shaderCompilerLoader.New();

	auto shaderSource = fileloading_utils::LoadStringFile("D:/Projects/Crimson/Build/x64/Debug/testShader.hlsl");

	auto spirVResult = pShaderCompiler->CompileShaderSource(EShaderSourceType::eHLSL
		, "testShader.hlsl"
		, shaderSource
		, "vert"
		, ECompileShaderType::eVert
		, false, true);


	std::shared_ptr<TestShaderProvider> vertProvider = std::make_shared<TestShaderProvider>();
	vertProvider->SetUniqueName("testShader.hlsl.vert");
	vertProvider->SetData("spirv", "vert", spirVResult.data(), spirVResult.size() * sizeof(uint32_t));


	spirVResult = pShaderCompiler->CompileShaderSource(EShaderSourceType::eHLSL
		, "testShader.hlsl"
		, shaderSource
		, "frag"
		, ECompileShaderType::eFrag
		, false, true);

	std::shared_ptr<TestShaderProvider> fragProvider = std::make_shared<TestShaderProvider>();
	fragProvider->SetUniqueName("testShader.hlsl.frag");
	fragProvider->SetData("spirv", "frag", spirVResult.data(), spirVResult.size() * sizeof(uint32_t));

	GraphicsShaderSet shaderSet{};
	shaderSet.vert = vertProvider;
	shaderSet.frag = fragProvider;


	auto pBackend = renderBackendLoader.New();
	pBackend->Initialize("Test Vulkan Backend", "CRIMSON Engine");
	pBackend->InitializeThreadContextCount(pThreadManager.get(), 1);
	auto windowHandle = pBackend->NewWindow(1024, 512, "Test Window");



	std::vector<VertexData> vertexDataList = {
		VertexData{-0.4f, 0.2f, 1.0f, 0.0f, 0.0f},
		VertexData{0.4f, 0.2f, 1.0f, 0.0f, 0.0f},
		VertexData{0.0f, 0.8f, 1.0f, 0.0f, 0.0f},
	};

	std::vector<VertexData> vertexDataList1 = {
		VertexData{-0.4f, -0.8f, 1.0f, 0.0f, 0.0f},
		VertexData{0.4f, -0.8f, 1.0f, 0.0f, 0.0f},
		VertexData{0.0f, -0.2f, 1.0f, 0.0f, 0.0f},
	};

	std::vector<uint16_t> indexDataList = {
		0, 1, 2
	};

	auto vertexBuffer = pBackend->CreateGPUBuffer(
		EBufferUsage::eVertexBuffer | EBufferUsage::eDataDst, 3, sizeof(VertexData));
	vertexBuffer->ScheduleBufferData(0, vertexDataList.size() * sizeof(VertexData), vertexDataList.data());

	auto vertexBuffer1 = pBackend->CreateGPUBuffer(
		EBufferUsage::eVertexBuffer | EBufferUsage::eDataDst, 3, sizeof(VertexData));
	vertexBuffer1->ScheduleBufferData(0, vertexDataList1.size() * sizeof(VertexData), vertexDataList1.data());

	auto indexBuffer = pBackend->CreateGPUBuffer(
		EBufferUsage::eIndexBuffer | EBufferUsage::eDataDst, 3, sizeof(uint16_t));
	indexBuffer->ScheduleBufferData(0, indexDataList.size() * sizeof(uint16_t), indexDataList.data());

	int32_t frame = 0;

	CVertexInputDescriptor vertexInputDesc{};
	vertexInputDesc.AddPrimitiveDescriptor(20, {
		VertexAttribute{0, offsetof(VertexData, x), VertexInputFormat::eR32G32_SFloat}
		, VertexAttribute{1, offsetof(VertexData, r), VertexInputFormat::eR32G32B32_SFloat}
		});


	auto windowBackBuffer = pRenderGraph->RegisterWindowBackbuffer(windowHandle);
	CAttachmentInfo attachmentInfo{};
	attachmentInfo.format = windowBackBuffer.GetDescriptor().format;
	attachmentInfo.loadOp = EAttachmentLoadOp::eClear;
	attachmentInfo.clearValue = GraphicsClearValue::ClearColor(0.0f, 1.0f, 1.0f, 1.0f);
	CRenderpassBuilder& newRenderPass = pRenderGraph->NewRenderPass({ attachmentInfo });
	newRenderPass.SetAttachmentTarget(0, windowBackBuffer);
	newRenderPass
		.Subpass({ {0} }, CPipelineStateObject{}, vertexInputDesc, shaderSet, [vertexBuffer, vertexBuffer1, indexBuffer](CInlineCommandList& cmd)
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
		})
		.Subpass({ {0} }, CPipelineStateObject{}, vertexInputDesc, shaderSet, [vertexBuffer1, indexBuffer](CInlineCommandList& cmd)
		{
			if (vertexBuffer1->UploadingDone() && indexBuffer->UploadingDone())
			{
				cmd.BindVertexBuffers({ vertexBuffer1.get() }, {});
				cmd.BindIndexBuffers(EIndexBufferType::e16, indexBuffer.get());
				cmd.DrawIndexed(3);
			}
			else
			{
				std::cout << "Not Finish Yet" << std::endl;
			}
		})
			;

	pRenderGraph->PresentWindow(windowHandle);

	while (pBackend->AnyWindowRunning())
	{
		pBackend->StartCurrentFrame();

		if (frame == 0)
		{
			vertexBuffer->DoUpload();
			vertexBuffer1->DoUpload();
			indexBuffer->DoUpload();
		}

		pBackend->ExecuteRenderGraph(pRenderGraph);
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
