#include <GPUDevice.h>
#include <headers/Win32Window.h>
#include <Compiler.h>
#include <fstream>
#include <iostream>
int main()
{
	using namespace Crimson;
	GPUDeviceManager::Init();
	GPUDeviceManager::Get()->InitAPIContext(EAPIType::E_API_TYPE_VULKAN, true);
	PGPUDevice MainDevice = GPUDeviceManager::Get()->CreateDevice("MainDevice", 0, EAPIType::E_API_TYPE_VULKAN, 3, 1, 1);
	PGPUDevice HelperDevice = GPUDeviceManager::Get()->CreateDevice("HelperDevice", 1, EAPIType::E_API_TYPE_VULKAN, 3, 1, 1);
	Win32Window new_window;
	new_window.InitWindow(L"Test Window", L"default", 1024, 720);
	MainDevice->RegisterWindow(new_window);

	PGPUBuffer test_buffer = MainDevice->CreateBuffer(256, { EBufferUsage::E_BUFFER_USAGE_UNIFORM }, EMemoryType::E_MEMORY_TYPE_DEVICE);
	test_buffer->Dispose();
	PGPUImage test_color = MainDevice->CreateImage(EFormat::E_FORMAT_B8G8R8A8_SRGB, 1024, 720, 1, { EImageUsage::E_IMAGE_USAGE_COLOR_ATTACHMENT }, EMemoryType::E_MEMORY_TYPE_DEVICE);
	PGPUImage test_depth_stencil = MainDevice->CreateImage(EFormat::E_FORMAT_D24_UNORM_S8_UINT, 1024, 720, 1, { EImageUsage::E_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT }, EMemoryType::E_MEMORY_TYPE_DEVICE);
	PRenderPass test_renderpass = MainDevice->CreateRenderPass();
	test_renderpass->m_Attachments = { {EFormat::E_FORMAT_B8G8R8A8_SRGB, EAttachmentClearType::E_ATTACHMENT_CLEAR_ZEROS}, {EFormat::E_FORMAT_D24_UNORM_S8_UINT, EAttachmentClearType::E_ATTACHMENT_CLEAR_ONES} };
	test_renderpass->m_Subpasses.resize(1);
	test_renderpass->m_Subpasses[0].m_OutputAttachments = { 0 };
	test_renderpass->m_Subpasses[0].m_DepthStencilAttachment = 1;
	test_renderpass->BuildRenderPass();

	PDescriptorSetLayout layout = MainDevice->CreateDescriptorSetLayout();
	layout->m_Bindings.resize(2);
	layout->m_Bindings[0].m_BindingPoint = 0;
	layout->m_Bindings[0].m_Num = 5;
	layout->m_Bindings[0].m_ResourceType = EShaderResourceType::E_SHADER_IMAGE_SAMPLER;
	layout->m_Bindings[0].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_VERTEX, EShaderType::E_SHADER_TYPE_FRAGMENT };

	layout->m_Bindings[1].m_BindingPoint = 1;
	layout->m_Bindings[1].m_Num = 1;
	layout->m_Bindings[1].m_ResourceType = EShaderResourceType::E_SHADER_UNIFORM_BUFFER;
	layout->m_Bindings[1].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_VERTEX, EShaderType::E_SHADER_TYPE_FRAGMENT };

	PGraphicsPipeline pipeline = MainDevice->CreateGraphicsPipeline();

	PDescriptorSet test_set = layout->AllocDescriptorSet();

	auto compiler = ShaderCompiler::IShaderCompiler::GetCompiler();

	{
		std::ifstream shader_src("test.vert");
		std::string src;
		if (shader_src.is_open())
		{
			std::string line;
			while (std::getline(shader_src, line))
			{
				src += line + "\n";
			}
		}
		src = compiler->PreprocessGLSLShader("test.vert", src, ShaderCompiler::ECompileShaderType::E_SHADER_TYPE_VERTEX);
		std::cout << src << std::endl;
		auto binary = compiler->CompileGLSLShaderSource("test.vert", src, ShaderCompiler::ECompileShaderType::E_SHADER_TYPE_VERTEX);
		pipeline->LoadShaderSource(reinterpret_cast<char*>(binary.data()), binary.size() * sizeof(uint32_t), EShaderType::E_SHADER_TYPE_VERTEX);
	}

	{
		std::ifstream shader_src("test.frag");
		std::string src;
		if (shader_src.is_open())
		{
			std::string line;
			while (std::getline(shader_src, line))
			{
				src += line + "\n";
			}
		}
		src = compiler->PreprocessGLSLShader("test.frag", src, ShaderCompiler::ECompileShaderType::E_SHADER_TYPE_FRAGMENT);
		std::cout << src << std::endl;
		auto binary = compiler->CompileGLSLShaderSource("test.frag", src, ShaderCompiler::ECompileShaderType::E_SHADER_TYPE_FRAGMENT);
		pipeline->LoadShaderSource(reinterpret_cast<char*>(binary.data()), binary.size() * sizeof(uint32_t), EShaderType::E_SHADER_TYPE_FRAGMENT);
	}
	pipeline->m_VertexInputs.resize(1);
	pipeline->m_VertexInputs[0].m_DataTypes = { EDataType::EVEC3 };
	pipeline->m_VertexInputs[0].m_VertexInputMode = EVertexInputMode::E_VERTEX_INPUT_PER_VERTEX;
	test_renderpass->InstanciatePipeline(pipeline, 0);

	PFramebuffer test_framebuffer = MainDevice->CreateFramebuffer();
	test_framebuffer->m_Images = { test_color , test_depth_stencil };

	PRenderPassInstance render_pass_instance = MainDevice->CreateRenderPassInstance(test_renderpass, test_framebuffer);

	PGPUDeviceThread test_thread = MainDevice->CreateThread();
	PGraphicsCommandBuffer cmd = test_thread->StartSubpassCommand(render_pass_instance, 0);
	cmd->BindSubpassPipeline(pipeline);

	cmd->EndCommandBuffer();

	while (new_window.IsWindowRunning())
	{
		new_window.UpdateWindow();
	}
	return 0;
}