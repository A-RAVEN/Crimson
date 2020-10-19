#define NOMINMAX
#include <GPUDevice.h>shaderGroupHandleSize
#include <headers/Win32Window.h>
#include <Compiler.h>
#include <fstream>
#include <array>
#include <assimp/scene.h>
//#include <headers/Resources/MeshletResource.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <headers/MeshResource.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <headers/stb_image.h>
#include <headers/VertexData.h>
#include <headers/KeyboardController.h>
//#include <headers/GeometryInstanceManager.h>
//#include <headers/BufferVector.h>
//#include <headers/MeshQueue.h>
#include <headers/TimeManager.h>
#include <headers/ShaderProcessor.h>
//#include <headers/ThreadManager.h>
#include <iostream>
#include <headers/Camera.h>
#include <Compiler.h>
//#include <headers/RenderingSystem.h>
//#include <headers/LuaInterface/LuaMachine.h>
//#include <headers/LuaInterface/LuaInterfaces.h>
//#include <headers/NetworkManager.h>
//#include <headers/Entity/World.h>
//#include <headers/Components/TransformComponent.h>
//#include <headers/Components/MeshRenderComp.h>
//#include <headers/Components/HierarchyComp.h>
//#include <headers/LuaInterface/GraphicsLuaMachine.h>

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
	PGPUDevice MainDevice = GPUDeviceManager::Get()->CreateDevice("MainDevice", 0, EAPIType::E_API_TYPE_D3D12, 3, 1, 1);
	auto dxCompiler = ShaderCompiler::IShaderCompiler::CreateDxCompiler();

	PShaderModule vsModule = nullptr;
	PShaderModule psModule = nullptr;
	{
		std::string file_name = "VertexShader.hlsl";
		std::ifstream shader_src(file_name);
		std::string src;
		std::vector<uint32_t> shaderByte;
		if (shader_src.is_open())
		{
			std::string line;
			while (std::getline(shader_src, line))
			{
				src += line + "\n";
			}
			shaderByte = dxCompiler->CompileGLSLShaderSource(file_name, src, ShaderCompiler::ECompileShaderType::E_SHADER_TYPE_VERTEX);
		}

		vsModule = MainDevice->CreateShaderModule(shaderByte.data(), shaderByte.size() * sizeof(uint32_t), EShaderType::E_SHADER_TYPE_VERTEX);
	}

	{
		std::string file_name = "PixelShader.hlsl";
		std::ifstream shader_src(file_name);
		std::string src;
		std::vector<uint32_t> shaderByte;
		if (shader_src.is_open())
		{
			std::string line;
			while (std::getline(shader_src, line))
			{
				src += line + "\n";
			}
			shaderByte = dxCompiler->CompileGLSLShaderSource(file_name, src, ShaderCompiler::ECompileShaderType::E_SHADER_TYPE_FRAGMENT);
		}

		psModule = MainDevice->CreateShaderModule(shaderByte.data(), shaderByte.size() * sizeof(uint32_t), EShaderType::E_SHADER_TYPE_FRAGMENT);
	}

	auto image = MainDevice->CreateImage(EFormat::E_FORMAT_B8G8R8A8_SRGB, 512, 512, 1, { EImageUsage::E_IMAGE_USAGE_COLOR_ATTACHMENT, EImageUsage::E_IMAGE_USAGE_COPY_SRC }, EMemoryType::E_MEMORY_TYPE_DEVICE);
	auto frameBuffer = MainDevice->CreateFramebuffer();
	frameBuffer->m_Images.push_back(image);
	frameBuffer->BuildFramebuffer();
	auto renderPass = MainDevice->CreateRenderPass();
	renderPass->m_Attachments.resize(1);
	renderPass->m_Attachments[0].m_Format = image->GetFormat();
	renderPass->m_Attachments[0].m_ClearType = EAttachmentClearType::E_ATTACHMENT_CLEAR_ZEROS;
	renderPass->m_Attachments[0].m_SampleCount = 1;

	renderPass->m_Subpasses.resize(1);
	renderPass->m_Subpasses[0].m_OutputAttachments = { 0 };
	renderPass->BuildRenderPass();
	PGPUBuffer buffer = MainDevice->CreateBuffer(sizeof(glm::mat4), { EBufferUsage::E_BUFFER_USAGE_UNIFORM }, EMemoryType::E_MEMORY_TYPE_HOST);

	PDescriptorSetLayout layout = MainDevice->CreateDescriptorSetLayout();
	layout->m_Bindings.resize(1);
	layout->m_Bindings[0].m_ResourceType = EShaderResourceType::E_SHADER_UNIFORM_BUFFER;
	layout->m_Bindings[0].m_Num = 1;
	layout->m_Bindings[0].m_RegisterPoint = 0;
	layout->m_Bindings[0].m_BindingPoint = 0;
	layout->m_Bindings[0].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_VERTEX };
	layout->BuildLayout();

	PDescriptorSet set = layout->AllocDescriptorSet();
	set->WriteDescriptorSetBuffers(0, { buffer }, { buffer->GetRange() }, 0);
	set->EndWriteDescriptorSet();

	PGraphicsPipeline pipeline = MainDevice->CreateGraphicsPipeline();
	//pipeline->m_DescriptorSetLayouts = { {0, layout} };
	pipeline->m_ShaderModules = { vsModule , psModule };
	pipeline->m_VertexInputs.resize(2);
	//pipeline->m_VertexInputs[0].m_DataTypes = rt_mesh->GetDataType();
	pipeline->m_VertexInputs[0].m_DataTypes = { EDataType::EVEC3 };
	pipeline->m_VertexInputs[0].m_VertexInputMode = EVertexInputMode::E_VERTEX_INPUT_PER_VERTEX;
	pipeline->m_VertexInputs[1].m_DataTypes = { EDataType::EUINT };
	pipeline->m_VertexInputs[1].m_VertexInputMode = EVertexInputMode::E_VERTEX_INPUT_PER_INSTANCE;
	pipeline->m_DepthRule = EDepthTestRule::E_DEPTH_TEST_ENABLED;
	pipeline->BuildPipeline();

	renderPass->InstanciatePipeline(pipeline, 0);

	auto renderInstance = MainDevice->CreateRenderPassInstance(renderPass, frameBuffer);

	PGPUDeviceThread device_thread = MainDevice->CreateThread();
	auto subpass_command = device_thread->StartSubpassCommand(renderInstance, 0);

	std::vector<glm::vec3> triangle{ glm::vec3(-0.8f, -0.8f, -0.1f), glm::vec3(0.8f, -0.8f, -0.1f) , glm::vec3(0.0f, 0.6f, -0.1f) };
	PGPUBuffer vertex_buffer = MainDevice->CreateBuffer(triangle.size() * sizeof(glm::vec3), { EBufferUsage::E_BUFFER_USAGE_VERTEX }, EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE);

	subpass_command->BindSubpassPipeline(pipeline);
	subpass_command->ViewPort(0.0f, 0.0f, 512.0f, 512.0f);
	subpass_command->Sissor(0, 0, 512, 512);
	subpass_command->BindVertexInputeBuffer({ vertex_buffer }, { vertex_buffer->GetRange() }, { sizeof(glm::vec3) });
	subpass_command->Draw(3, 1, 0, 0);



	Win32Window new_window;
	new_window.InitWindow(L"Test Window", L"default", 1024, 720);
	MainDevice->RegisterWindow(new_window);
	while (new_window.IsWindowRunning())
	{
		new_window.UpdateWindow();
	}
	GPUDeviceManager::Get()->Dispose();
}