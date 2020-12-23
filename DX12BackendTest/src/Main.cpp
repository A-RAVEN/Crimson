#define NOMINMAX
#include <GPUDevice.h>
#include <headers/Win32Window.h>
#include <Compiler.h>
#include <fstream>
#include <array>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <headers/MeshResource.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <headers/stb_image.h>
#include <headers/VertexData.h>
#include <headers/KeyboardController.h>
#include <headers/TimeManager.h>
#include <headers/ShaderProcessor.h>
#include <iostream>
#include <headers/Camera.h>
#include <Compiler.h>
#include <stdint.h>

int main()
{
	int x,y,n;
    unsigned char *data = stbi_load("ExportValue.png", &x, &y, &n, 4);
	int idy = y / 2;
	for (int i = 0; i < x / 2; ++i)
	{
		float a = 1.0f - (i * 2.0f / x);
		unsigned char alpha = data[4 * (idy * y + i) + 3];
		float val = alpha / 255.0f;
		std::cout << a << ", " << val << std::endl;
	}
	using namespace Crimson;
	GPUDeviceManager::Init();
	std::cout << "Init Context" << std::endl;
	GPUDeviceManager::Get()->InitAPIContext(EAPIType::E_API_TYPE_D3D12, true);
	std::cout << "Create Device" << std::endl;
	PGPUDevice MainDevice = GPUDeviceManager::Get()->CreateDevice("MainDevice", 0, EAPIType::E_API_TYPE_D3D12, 1, 1, 1);
	auto dxCompiler = ShaderCompiler::IShaderCompiler::CreateDxCompiler();

	PShaderModule vsModule = nullptr;
	PShaderModule psModule = nullptr;
	{
		std::string file_name = "VertexShader.hlsl";
		std::ifstream shader_src(file_name);
		std::string src;
		std::vector<char> shaderByte;
		if (shader_src.is_open())
		{
			std::string line;
			while (std::getline(shader_src, line))
			{
				src += line + "\n";
			}
			shaderByte = dxCompiler->CompileHLSLShaderSource(file_name, src, ShaderCompiler::ECompileShaderType::E_SHADER_TYPE_VERTEX);
			shader_src.close();
		}
		vsModule = MainDevice->CreateShaderModule(shaderByte, EShaderType::E_SHADER_TYPE_VERTEX);
	}

	{
		std::string file_name = "PixelShader.hlsl";
		std::ifstream shader_src(file_name);
		std::string src;
		std::vector<char> shaderByte;
		if (shader_src.is_open())
		{
			std::string line;
			while (std::getline(shader_src, line))
			{
				src += line + "\n";
			}
			shaderByte = dxCompiler->CompileHLSLShaderSource(file_name, src, ShaderCompiler::ECompileShaderType::E_SHADER_TYPE_FRAGMENT);
			shader_src.close();
		}

		psModule = MainDevice->CreateShaderModule(shaderByte, EShaderType::E_SHADER_TYPE_FRAGMENT);
	}

	auto image = MainDevice->CreateImage(EFormat::E_FORMAT_R8G8B8A8_UNORM, 512, 512, 1, { EImageUsage::E_IMAGE_USAGE_COLOR_ATTACHMENT, EImageUsage::E_IMAGE_USAGE_COPY_SRC }, EMemoryType::E_MEMORY_TYPE_DEVICE);
	MainDevice->Diagnose();
	auto frameBuffer = MainDevice->CreateFramebuffer();
	frameBuffer->m_Images.push_back(image);
	frameBuffer->BuildFramebuffer();
	MainDevice->Diagnose();
	auto renderPass = MainDevice->CreateRenderPass();
	renderPass->m_Attachments.resize(1);
	renderPass->m_Attachments[0].m_Format = image->GetFormat();
	renderPass->m_Attachments[0].m_ClearType = EAttachmentClearType::E_ATTACHMENT_CLEAR_ZEROS;
	renderPass->m_Attachments[0].m_SampleCount = 1;

	renderPass->m_Subpasses.resize(1);
	renderPass->m_Subpasses[0].m_OutputAttachments = { 0 };
	renderPass->m_Subpasses[0].m_ViewportSetting = { 0.0f, 0.0f, 512.0f, 512.0f };
	renderPass->m_Subpasses[0].m_SissorsSetting = { 0.0f, 0.0f, 512.0f, 512.0f };
	renderPass->BuildRenderPass();
	MainDevice->Diagnose();
	PGPUBuffer buffer = MainDevice->CreateBuffer(sizeof(glm::mat4), { EBufferUsage::E_BUFFER_USAGE_UNIFORM }, EMemoryType::E_MEMORY_TYPE_DEVICE);

	PDescriptorSetLayout layout = MainDevice->CreateDescriptorSetLayout();
	layout->m_Bindings.resize(1);
	layout->m_Bindings[0].m_ResourceType = EShaderResourceType::E_SHADER_UNIFORM_BUFFER;
	layout->m_Bindings[0].m_Num = 1;
	layout->m_Bindings[0].m_RegisterPoint = 0;
	layout->m_Bindings[0].m_BindingPoint = 0;
	layout->m_Bindings[0].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_VERTEX };
	layout->BuildLayout();

	PDescriptorSet set = layout->AllocDescriptorSet();
	//set->WriteDescriptorSetBuffers(0, { buffer }, { buffer->GetRange() }, 0);
	set->EndWriteDescriptorSet();

	PGraphicsPipeline pipeline = MainDevice->CreateGraphicsPipeline();
	//pipeline->m_DescriptorSetLayouts = { {0, layout} };
	pipeline->m_ShaderModules = { vsModule , psModule };
	pipeline->m_VertexInputs.resize(1);
	//pipeline->m_VertexInputs[0].m_DataTypes = rt_mesh->GetDataType();
	pipeline->m_VertexInputs[0].m_DataTypes = { EDataType::EVEC3 };
	pipeline->m_VertexInputs[0].m_VertexInputMode = EVertexInputMode::E_VERTEX_INPUT_PER_VERTEX;
	pipeline->m_VertexInputs[0].m_Sematics = { "POSITION" };
	//pipeline->m_VertexInputs[1].m_DataTypes = { EDataType::EUINT };
	//pipeline->m_VertexInputs[1].m_VertexInputMode = EVertexInputMode::E_VERTEX_INPUT_PER_INSTANCE;
	pipeline->m_DepthRule = EDepthTestRule::E_DEPTH_TEST_ENABLED;
	pipeline->m_Topology = ETopology::E_TOPOLOGY_TRIANGLE_LIST;
	pipeline->BuildPipeline();

	renderPass->InstanciatePipeline(pipeline, 0);

	auto renderInstance = MainDevice->CreateRenderPassInstance(renderPass, frameBuffer);
	renderInstance->ConfigureViewPort(0.0f, 0.0f, 512.0f, 512.0f);
	renderInstance->ConfigureSissor(0.0f, 0.0f, 512.0f, 512.0f);

	MainDevice->CreateBatch("RenderBatch", EExecutionCommandType::E_COMMAND_TYPE_GENERAL, 0);
	MainDevice->CreateBatch("RenderBatch1", EExecutionCommandType::E_COMMAND_TYPE_GENERAL, 0);
	PGPUDeviceThread device_thread = MainDevice->CreateThread();
	auto subpass_command = device_thread->StartSubpassCommand(renderInstance, 0);

	std::vector<glm::vec3> triangle{ glm::vec3(-0.8f, -0.8f, 0.1f), glm::vec3(0.8f, -0.8f, 0.1f) , glm::vec3(0.0f, 0.6f, 0.1f) };
	PGPUBuffer vertex_buffer = MainDevice->CreateBuffer(triangle.size() * sizeof(glm::vec3), { EBufferUsage::E_BUFFER_USAGE_VERTEX }, EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE);
	memcpy(vertex_buffer->GetMappedPointer(), triangle.data(), triangle.size() * sizeof(glm::vec3));
	vertex_buffer->UnMapp();
	subpass_command->BindSubpassPipeline(pipeline);
	//subpass_command->ViewPort(0.0f, 0.0f, 512.0f, 512.0f);
	//subpass_command->Sissor(0, 0, 512, 512);
	subpass_command->BindVertexInputeBuffer({ vertex_buffer }, { vertex_buffer->GetRange() }, { sizeof(glm::vec3) });
	subpass_command->Draw(3, 1, 0, 0);
	subpass_command->EndCommandBuffer();

	auto executeCmd = device_thread->CreateExecutionCommandBuffer(EExecutionCommandType::E_COMMAND_TYPE_GENERAL);
	device_thread->BindExecutionCommandBufferToBatch("RenderBatch", executeCmd);
	auto executeCmd1 = device_thread->CreateExecutionCommandBuffer(EExecutionCommandType::E_COMMAND_TYPE_GENERAL);
	device_thread->BindExecutionCommandBufferToBatch("RenderBatch1", executeCmd1);

	Win32Window new_window;
	Win32Window new_window1;
	new_window.InitWindow(L"Test Window", L"default", 512, 512);
	new_window1.InitWindow(L"Test Window1", L"default", 512, 512);
	MainDevice->RegisterWindow(new_window);
	MainDevice->RegisterWindow(new_window1);
	bool presentOne = false;
	while (new_window.IsWindowRunning()&& new_window1.IsWindowRunning())
	{
		executeCmd->StartCommand();
		executeCmd->ExecuteRenderPassInstance(renderInstance);
		executeCmd->CopyToSwapchain_Dynamic(image, &new_window);
		executeCmd->EndCommand();

		executeCmd1->StartCommand();
		executeCmd1->ExecuteRenderPassInstance(renderInstance);
		executeCmd1->CopyToSwapchain_Dynamic(image, &new_window1);
		executeCmd1->EndCommand();

		MainDevice->ExecuteBatches({ "RenderBatch", "RenderBatch1" }, EExecutionCommandType::E_COMMAND_TYPE_GENERAL, 0);
		MainDevice->PresentWindow(new_window);
		MainDevice->PresentWindow(new_window1);
		if (presentOne)
		{
		}
		else
		{
		}
		presentOne = !presentOne;
		new_window.UpdateWindow();
		new_window1.UpdateWindow();
		MainDevice->Diagnose();
	}
	GPUDeviceManager::Get()->Dispose();
}