#define NOMINMAX
#include <GPUDevice.h>
#include <headers/Win32Window.h>
#include <Compiler.h>
#include <fstream>
#include <iostream>
#include <array>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <headers/MeshResource.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
int main()
{
	Assimp::Importer scene_importer;
	const aiScene* scene = scene_importer.ReadFile("testslime.obj", aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	std::cout << "Start" << std::endl;
	using namespace Crimson;
	GPUDeviceManager::Init();
	std::cout << "Init Context" << std::endl;
	GPUDeviceManager::Get()->InitAPIContext(EAPIType::E_API_TYPE_VULKAN, true);
	std::cout << "Create Device" << std::endl;
	PGPUDevice MainDevice = GPUDeviceManager::Get()->CreateDevice("MainDevice", 0, EAPIType::E_API_TYPE_VULKAN, 3, 1, 1);

	MeshResource new_resource;
	new_resource.ProcessAiScene(scene);

	//Try Create Ray Tracing Structure
	PRayTracer raytracer = MainDevice->CreateRayTracer();
	PRayTraceGeometry raytrace_geometry = MainDevice->CreateRayTraceGeometry();
	PAccelerationStructure accel_struct = MainDevice->CreateAccelerationStructure();
	raytrace_geometry->SetVertexData(new_resource.m_VertexBuffer, 0, new_resource.m_VertexSize, 0, EDataType::EVEC3);
	accel_struct->m_Geometries = { raytrace_geometry };
	accel_struct->m_BuildFlags = { EBuildAccelerationStructureFlags::E_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV };
	accel_struct->InitAS();

	PAccelerationStructure tlas = MainDevice->CreateAccelerationStructure();
	tlas->m_InstanceNumber = 1;
	tlas->m_BuildFlags = { EBuildAccelerationStructureFlags::E_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV };
	tlas->InitAS(true);

	//PGPUDevice HelperDevice = GPUDeviceManager::Get()->CreateDevice("HelperDevice", 1, EAPIType::E_API_TYPE_VULKAN, 3, 1, 1);
	Win32Window new_window;
	new_window.InitWindow(L"Test Window", L"default", 1024, 720);
	MainDevice->RegisterWindow(new_window);

	std::cout << "Window" << std::endl;


	PGPUBuffer test_buffer = MainDevice->CreateBuffer(256, { EBufferUsage::E_BUFFER_USAGE_UNIFORM }, EMemoryType::E_MEMORY_TYPE_DEVICE);
	test_buffer->Dispose();
	
	PGPUBuffer vertex_buffer = MainDevice->CreateBuffer(sizeof(float) * 3 * 3, { EBufferUsage::E_BUFFER_USAGE_VERTEX }, EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE);

	PGPUBuffer camera_buffer = MainDevice->CreateBuffer(sizeof(glm::mat4), { EBufferUsage::E_BUFFER_USAGE_UNIFORM }, EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE);

	PGPUBuffer geometry_instance_buffer = MainDevice->CreateBuffer(sizeof(RayTraceGeometryInstance), { EBufferUsage::E_BUFFER_USAGE_RAYTRACING_NV }, EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE);
	RayTraceGeometryInstance* pGeometryInstance = new (geometry_instance_buffer->GetMappedPointer()) RayTraceGeometryInstance();
	pGeometryInstance->m_Flags = static_cast<uint32_t>(EGeometryInstanceFlags::E_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE);
	pGeometryInstance->m_InstanceId = 0;
	pGeometryInstance->m_Mask = 0xff;
	pGeometryInstance->m_AccelerationStructureHandle = accel_struct->GetHandle();
	*(reinterpret_cast<glm::mat3x4*>(&pGeometryInstance->m_TransformMatrix)) = 
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
	};

	glm::mat4 change = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f));
	memcpy(camera_buffer->GetMappedPointer(), &change, sizeof(glm::mat4));
	camera_buffer->UnMapp();

	std::array<float, 9> triangle_data = {
		1.0f, -1.0f, 0.1f,
		-1.0f, -1.0f, 0.1f,
		0.0f, 1.0f, 0.1f
	};

	memcpy(vertex_buffer->GetMappedPointer(), triangle_data.data(), triangle_data.size() * sizeof(float));

	PGPUImage test_color = MainDevice->CreateImage(EFormat::E_FORMAT_B8G8R8A8_SRGB, 1024, 720, 1, { EImageUsage::E_IMAGE_USAGE_COLOR_ATTACHMENT, EImageUsage::E_IMAGE_USAGE_COPY_SRC }, EMemoryType::E_MEMORY_TYPE_DEVICE);
	PGPUImage test_rtimg = MainDevice->CreateImage(EFormat::E_FORMAT_B8G8R8A8_UNORM, 1024, 720, 1, { EImageUsage::E_IMAGE_USAGE_COPY_SRC, EImageUsage::E_IMAGE_USAGE_STORAGE, EImageUsage::E_IMAGE_USAGE_SAMPLE }, EMemoryType::E_MEMORY_TYPE_DEVICE);
	PGPUImage test_depth_stencil = MainDevice->CreateImage(EFormat::E_FORMAT_D24_UNORM_S8_UINT, 1024, 720, 1, { EImageUsage::E_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT }, EMemoryType::E_MEMORY_TYPE_DEVICE);
	PRenderPass test_renderpass = MainDevice->CreateRenderPass();
	test_renderpass->m_Attachments = { {EFormat::E_FORMAT_B8G8R8A8_SRGB, EAttachmentClearType::E_ATTACHMENT_CLEAR_ZEROS} , { EFormat::E_FORMAT_D24_UNORM_S8_UINT, EAttachmentClearType::E_ATTACHMENT_CLEAR_ONES }};
	test_renderpass->m_Subpasses.resize(1);
	test_renderpass->m_Subpasses[0].m_OutputAttachments = { 0 };
	test_renderpass->m_Subpasses[0].m_DepthStencilAttachment = 1;
	test_renderpass->BuildRenderPass();

	PDescriptorSetLayout layout = MainDevice->CreateDescriptorSetLayout();
	layout->m_Bindings.resize(1);
	layout->m_Bindings[0].m_BindingPoint = 0;
	layout->m_Bindings[0].m_Num = 1;
	layout->m_Bindings[0].m_ResourceType = EShaderResourceType::E_SHADER_UNIFORM_BUFFER;
	layout->m_Bindings[0].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_VERTEX, EShaderType::E_SHADER_TYPE_FRAGMENT };

	PDescriptorSetLayout rtlayout = MainDevice->CreateDescriptorSetLayout();
	rtlayout->m_Bindings.resize(3);
	rtlayout->m_Bindings[0].m_BindingPoint = 0;
	rtlayout->m_Bindings[0].m_Num = 1;
	rtlayout->m_Bindings[0].m_ResourceType = EShaderResourceType::E_SHADER_UNIFORM_BUFFER;
	rtlayout->m_Bindings[0].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_RAYGEN_NV };

	rtlayout->m_Bindings[1].m_BindingPoint = 1;
	rtlayout->m_Bindings[1].m_Num = 1;
	rtlayout->m_Bindings[1].m_ResourceType = EShaderResourceType::E_SHADER_ACCEL_STRUCT_NV;
	rtlayout->m_Bindings[1].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_RAYGEN_NV };

	rtlayout->m_Bindings[2].m_BindingPoint = 2;
	rtlayout->m_Bindings[2].m_Num = 1;
	rtlayout->m_Bindings[2].m_ResourceType = EShaderResourceType::E_SHADER_STORAGE_IMAGE;
	rtlayout->m_Bindings[2].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_RAYGEN_NV };
	rtlayout->BuildLayout();

	PDescriptorSet set = layout->AllocDescriptorSet();
	set->WriteDescriptorSetBuffers(0, { camera_buffer }, { {0, sizeof(glm::mat4)} }, 0);
	set->EndWriteDescriptorSet();

	PDescriptorSet rtset = rtlayout->AllocDescriptorSet();
	rtset->WriteDescriptorSetBuffers(0, { camera_buffer }, { {0, sizeof(glm::mat4)} }, 0);
	rtset->WriteDescriptorSetAccelStructuresNV(1, { tlas });
	rtset->WriteDescriptorSetImage(2, test_rtimg, EFilterMode::E_FILTER_MODE_NEAREST, EAddrMode::E_ADDR_MIRRORED_REPEAT, EViewAsType::E_VIEW_AS_COLOR);
	rtset->EndWriteDescriptorSet();

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
	pipeline->m_DepthRule = EDepthTestRule::E_DEPTH_TEST_ENABLED;
	pipeline->m_StencilRule = EStencilRule::E_STENCIL_WRITE;
	pipeline->m_DescriptorSetLayouts.push_back({ 0, layout });
	test_renderpass->InstanciatePipeline(pipeline, 0);

	{
		std::ifstream shader_src("raytrace.rgen");
		std::string src;
		if (shader_src.is_open())
		{
			std::string line;
			while (std::getline(shader_src, line))
			{
				src += line + "\n";
			}
		}
		src = compiler->PreprocessGLSLShader("raytrace.rgen", src, ShaderCompiler::ECompileShaderType::E_SHADER_TYPE_RAYGEN_NV);
		std::cout << src << std::endl;
		auto binary = compiler->CompileGLSLShaderSource("raytrace.rgen", src, ShaderCompiler::ECompileShaderType::E_SHADER_TYPE_RAYGEN_NV);
		raytracer->LoadShaderSource(reinterpret_cast<char*>(binary.data()), binary.size() * sizeof(uint32_t), EShaderType::E_SHADER_TYPE_RAYGEN_NV);
	}
	raytracer->m_DescriptorSetLayouts.push_back({ 0, rtlayout });
	raytracer->m_MaxRecursionDepth = 1;
	raytracer->Build();

	PFramebuffer test_framebuffer = MainDevice->CreateFramebuffer();
	test_framebuffer->m_Images = { test_color, test_depth_stencil};

	PRenderPassInstance render_pass_instance = MainDevice->CreateRenderPassInstance(test_renderpass, test_framebuffer);

	MainDevice->CreateBatch("GraphicsLoading", EExecutionCommandType::E_COMMAND_TYPE_GRAPHICS, 0);
	MainDevice->CreateBatch("Main Render", EExecutionCommandType::E_COMMAND_TYPE_GRAPHICS, 0);
	MainDevice->CreateBatch("Present", EExecutionCommandType::E_COMMAND_TYPE_GRAPHICS, 0);

	PGPUDeviceThread test_thread = MainDevice->CreateThread();

	PExecutionCommandBuffer execution = test_thread->CreateExecutionCommandBuffer(EExecutionCommandType::E_COMMAND_TYPE_GRAPHICS);
	test_thread->BindExecutionCommandBufferToBatch("Main Render", execution);

	PExecutionCommandBuffer present = test_thread->CreateExecutionCommandBuffer(EExecutionCommandType::E_COMMAND_TYPE_GRAPHICS);
	test_thread->BindExecutionCommandBufferToBatch("Present", present);

	PExecutionCommandBuffer load = test_thread->CreateExecutionCommandBuffer(EExecutionCommandType::E_COMMAND_TYPE_GRAPHICS);
	load->StartCommand();
	load->BuildAccelerationStructure(accel_struct);
	load->BuildAccelerationStructure(tlas, geometry_instance_buffer);
	load->EndCommand();
	test_thread->BindExecutionCommandBufferToBatch("GraphicsLoading", load, true);

	PGraphicsCommandBuffer cmd = test_thread->StartSubpassCommand(render_pass_instance, 0);
	cmd->ViewPort(0.0f, 0.0f, 1024.0f, 720.0f);
	cmd->Sissor(0, 0, 1024, 720);
	cmd->BindSubpassPipeline(pipeline);
	cmd->BindSubpassDescriptorSets({ set });
	cmd->BindVertexInputeBuffer({ new_resource.m_VertexBuffer }, { 0 });
	cmd->BindIndexBuffer(new_resource.m_IndexBuffer, 0);
	cmd->DrawIndexed(new_resource.m_IndexSize, 1);
	//cmd->Draw(3, 1, 0, 0);
	cmd->EndCommandBuffer();


	while (new_window.IsWindowRunning())
	{
		execution->StartCommand();
		execution->ExecuteRenderPassInstance(render_pass_instance);
		execution->EndCommand();

		present->StartCommand();
		present->CopyToSwapchain_Dynamic(test_color, &new_window);
		present->EndCommand();

		MainDevice->ExecuteBatches({ "GraphicsLoading", "Main Render", "Present" });

		MainDevice->PresentWindow(new_window);
		new_window.UpdateWindow();
	}
	return 0;
}