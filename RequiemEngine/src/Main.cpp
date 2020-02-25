#define NOMINMAX
#include <GPUDevice.h>shaderGroupHandleSize
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
#include <headers/stb_image.h>
#include <headers/VertexData.h>
#include <headers/KeyboardController.h>
#include <headers/GeometryInstanceManager.h>
#include <headers/BufferVector.h>

struct Camera
{
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 viewInverse;
	glm::mat4 projInverse;
};

int main()
{
	Camera cam;
	cam.view = glm::lookAt(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	cam.proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	cam.viewInverse = glm::inverse(cam.view);
	cam.projInverse = glm::inverse(cam.proj);


	Assimp::Importer scene_importer;
	const aiScene* scene = scene_importer.ReadFile("testslime.obj", aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	std::cout << "Start" << std::endl;
	using namespace Crimson;
	GPUDeviceManager::Init();
	std::cout << "Init Context" << std::endl;
	GPUDeviceManager::Get()->InitAPIContext(EAPIType::E_API_TYPE_VULKAN, true);
	std::cout << "Create Device" << std::endl;
	PGPUDevice MainDevice = GPUDeviceManager::Get()->CreateDevice("MainDevice", 0, EAPIType::E_API_TYPE_VULKAN, 3, 1, 1);

	BufferQueue<int, 500> test_queue;
	test_queue.Init(MainDevice, { EBufferUsage::E_BUFFER_USAGE_STORAGE }, EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE);
	test_queue.PushBack(1);

	TransformManager m_TransformManager;

	TransformComponent* p_component = m_TransformManager.AllocateTransformComponent();
	p_component->m_RawPointer->m_ModelTransform = glm::mat4(1.0f);

	MeshResource new_resource;
	new_resource.ProcessAiScene(scene);

	//Try Create Ray Tracing Structure
	PRayTracer raytracer = MainDevice->CreateRayTracer();
	PRayTraceGeometry raytrace_geometry = MainDevice->CreateRayTraceGeometry();
	PAccelerationStructure accel_struct = MainDevice->CreateAccelerationStructure();
	raytrace_geometry->SetVertexData(new_resource.m_VertexBuffer, 0, new_resource.m_VertexSize, sizeof(VertexDataLightWeight), EDataType::EVEC3);
	raytrace_geometry->SetIndexData(new_resource.m_IndexBuffer, 0, new_resource.m_IndexSize, EIndexType::E_INDEX_TYPE_32);
	raytrace_geometry->SetGeometryFlags({ EGeometryFlags::E_GEOMETRY_OPAQUE });
	raytrace_geometry->SetGeometryType(ERayTraceGeometryType::E_GEOMETRY_TYPE_TRIANGLES);
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

	PGPUBuffer camera_buffer = MainDevice->CreateBuffer(sizeof(Camera), { EBufferUsage::E_BUFFER_USAGE_UNIFORM }, EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE);

	PGPUBuffer geometry_instance_buffer = MainDevice->CreateBuffer(sizeof(RayTraceGeometryInstance), { EBufferUsage::E_BUFFER_USAGE_RAYTRACING_NV }, EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE);
	RayTraceGeometryInstance* pGeometryInstance = new (geometry_instance_buffer->GetMappedPointer()) RayTraceGeometryInstance();
	pGeometryInstance->m_Flags = static_cast<uint32_t>(EGeometryInstanceFlags::E_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE);
	pGeometryInstance->m_InstanceId = 0;
	pGeometryInstance->m_Mask = 0x1;// 0xff;
	pGeometryInstance->m_AccelerationStructureHandle = accel_struct->GetHandle();
	*(reinterpret_cast<glm::mat3x4*>(&pGeometryInstance->m_TransformMatrix)) = 
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
	};

	memcpy(camera_buffer->GetMappedPointer(), &cam, sizeof(Camera));
	//camera_buffer->UnMapp();

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
	layout->m_Bindings.resize(2);
	layout->m_Bindings[0].m_BindingPoint = 0;
	layout->m_Bindings[0].m_Num = 1;
	layout->m_Bindings[0].m_ResourceType = EShaderResourceType::E_SHADER_UNIFORM_BUFFER;
	layout->m_Bindings[0].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_VERTEX, EShaderType::E_SHADER_TYPE_FRAGMENT };
	layout->m_Bindings[1].m_BindingPoint = 1;
	layout->m_Bindings[1].m_Num = 1;
	layout->m_Bindings[1].m_ResourceType = EShaderResourceType::E_SHADER_IMAGE_SAMPLER;
	layout->m_Bindings[1].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_FRAGMENT };

	PDescriptorSetLayout rtlayout = MainDevice->CreateDescriptorSetLayout();
	rtlayout->m_Bindings.resize(3);
	rtlayout->m_Bindings[0].m_BindingPoint = 0;
	rtlayout->m_Bindings[0].m_Num = 1;
	rtlayout->m_Bindings[0].m_ResourceType = EShaderResourceType::E_SHADER_UNIFORM_BUFFER;
	rtlayout->m_Bindings[0].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_RAYGEN_NV };

	rtlayout->m_Bindings[1].m_BindingPoint = 1;
	rtlayout->m_Bindings[1].m_Num = 1;
	rtlayout->m_Bindings[1].m_ResourceType = EShaderResourceType::E_SHADER_ACCEL_STRUCT_NV;
	rtlayout->m_Bindings[1].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_RAYGEN_NV, EShaderType::E_SHADER_TYPE_MISS_NV, EShaderType::E_SHADER_TYPE_CLOSEHIT_NV };

	rtlayout->m_Bindings[2].m_BindingPoint = 2;
	rtlayout->m_Bindings[2].m_Num = 1;
	rtlayout->m_Bindings[2].m_ResourceType = EShaderResourceType::E_SHADER_STORAGE_IMAGE;
	rtlayout->m_Bindings[2].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_RAYGEN_NV };
	rtlayout->BuildLayout();

	PDescriptorSet set = layout->AllocDescriptorSet();
	set->WriteDescriptorSetBuffers(0, { camera_buffer }, { {0, sizeof(glm::mat4)} }, 0);
	set->EndWriteDescriptorSet();

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
	//pipeline->m_VertexInputs[0].m_DataTypes = { EDataType::EVEC3 };
	//pipeline->m_VertexInputs[0].m_VertexInputMode = EVertexInputMode::E_VERTEX_INPUT_PER_VERTEX;
	pipeline->m_VertexInputs[0].m_DataTypes = VertexDataLightWeight::GetDataType();
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
		raytracer->LoadShaderSource(reinterpret_cast<char*>(binary.data()), binary.size() * sizeof(uint32_t), EShaderType::E_SHADER_TYPE_RAYGEN_NV, "default");
	}

	{
		std::ifstream shader_src("closehit.rchit");
		std::string src;
		if (shader_src.is_open())
		{
			std::string line;
			while (std::getline(shader_src, line))
			{
				src += line + "\n";
			}
		}
		src = compiler->PreprocessGLSLShader("closehit.rchit", src, ShaderCompiler::ECompileShaderType::E_SHADER_TYPE_CLOSEHIT_NV);
		std::cout << src << std::endl;
		auto binary = compiler->CompileGLSLShaderSource("closehit.rchit", src, ShaderCompiler::ECompileShaderType::E_SHADER_TYPE_CLOSEHIT_NV);
		raytracer->LoadShaderSource(reinterpret_cast<char*>(binary.data()), binary.size() * sizeof(uint32_t), EShaderType::E_SHADER_TYPE_CLOSEHIT_NV, "default");
	}
	{
		std::ifstream shader_src("miss.rmiss");
		std::string src;
		if (shader_src.is_open())
		{
			std::string line;
			while (std::getline(shader_src, line))
			{
				src += line + "\n";
			}
		}
		src = compiler->PreprocessGLSLShader("miss.rmiss", src, ShaderCompiler::ECompileShaderType::E_SHADER_TYPE_MISS_NV);
		std::cout << src << std::endl;
		auto binary = compiler->CompileGLSLShaderSource("miss.rmiss", src, ShaderCompiler::ECompileShaderType::E_SHADER_TYPE_MISS_NV);
		raytracer->LoadShaderSource(reinterpret_cast<char*>(binary.data()), binary.size() * sizeof(uint32_t), EShaderType::E_SHADER_TYPE_MISS_NV, "default");
	}
	raytracer->m_DescriptorSetLayouts.push_back({ 0, rtlayout });
	raytracer->m_MaxRecursionDepth = 5;
	raytracer->Build();
	PGPUBuffer shader_table = MainDevice->CreateBuffer(raytracer->GetShaderTableSize("default"), {EBufferUsage::E_BUFFER_USAGE_RAYTRACING_NV}, EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE);
	raytracer->CopyShaderTable(shader_table->GetMappedPointer(), "default");

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

	PGPUBuffer image_load_buffer;
	PGPUImage test_loaded_image;
	{
		int w, h, channel_num;
		auto data = stbi_load("testnormal1.jpg", &w, &h, &channel_num, 4);
		image_load_buffer = MainDevice->CreateBuffer(w * h * sizeof(int), { EBufferUsage::E_BUFFER_USAGE_COPY_SRC }, EMemoryType::E_MEMORY_TYPE_HOST);
		memcpy(image_load_buffer->GetMappedPointer(), data, w* h * sizeof(int));
		stbi_image_free(data);
		image_load_buffer->UnMapp();
		test_loaded_image = MainDevice->CreateImage(EFormat::E_FORMAT_R8G8B8A8_UNORM, w, h, 1, { EImageUsage::E_IMAGE_USAGE_COPY_DST, EImageUsage::E_IMAGE_USAGE_SAMPLE }
		, EMemoryType::E_MEMORY_TYPE_DEVICE);

		set->WriteDescriptorSetImage(1, test_loaded_image, EFilterMode::E_FILTER_MODE_LINEAR, EAddrMode::E_ADDR_MIRRORED_REPEAT);
		set->EndWriteDescriptorSet();
	}

	PExecutionCommandBuffer load = test_thread->CreateExecutionCommandBuffer(EExecutionCommandType::E_COMMAND_TYPE_GRAPHICS);
	load->StartCommand();
	load->CopyBufferToImage(image_load_buffer, test_loaded_image);
	load->BuildAccelerationStructure(accel_struct);
	load->DeviceMemoryBarrier(EMemoryBarrierType::E_ACCEL_STRUCTURE_BUILD_READ_WRITE);
	load->BuildAccelerationStructure(tlas, geometry_instance_buffer);
	load->DeviceMemoryBarrier(EMemoryBarrierType::E_ACCEL_STRUCTURE_BUILD_READ_WRITE);
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

	MainDevice->ExecuteBatches({ "GraphicsLoading" });
	MainDevice->WaitIdle();

	PDescriptorSet rtset = rtlayout->AllocDescriptorSet();
	rtset->WriteDescriptorSetBuffers(0, { camera_buffer }, { {0, sizeof(glm::mat4)} }, 0);
	rtset->WriteDescriptorSetAccelStructuresNV(1, { tlas });
	rtset->WriteDescriptorSetImage(2, test_rtimg, EFilterMode::E_FILTER_MODE_NEAREST, EAddrMode::E_ADDR_MIRRORED_REPEAT, EViewAsType::E_VIEW_AS_COLOR);
	rtset->EndWriteDescriptorSet();

	KeyboardController inputs;

	glm::vec2 angles(0.0f);
	glm::vec3 position(-5.0f, 0.0f, 0.0f);

	bool toggle = false;

	while (new_window.IsWindowRunning())
	{
		inputs.UpdateController();
		//for (int i = 0; i < 10; ++i)
		//{
		//	if (inputs.KeyTriggered(inputs.GetNumKey(i)))
		//	{
		//		std::cout << i << std::endl;
		//	}
		//}
		//for (char itr_c = 'A'; itr_c <= 'Z'; ++itr_c)
		//{
		//	if (inputs.KeyTriggered(inputs.GetCharKey(itr_c)))
		//	{
		//		std::cout << itr_c << std::endl;
		//	}
		//}

		if (inputs.KeyTriggered(inputs.GetCharKey('C')))
		{
			toggle = !toggle;
		}

		if (inputs.KeyState(VK_RBUTTON))
		{
			glm::vec2 movement = inputs.GetMouseMovement();
			angles.x = (angles.x + movement.x * 0.01f);
			angles.y = glm::clamp(angles.y + movement.y * 0.01f, -glm::pi<float>() * 0.49999f, glm::pi<float>() * 0.49999f);
			//std::cout << angles.x << " " << angles.y << std::endl;
		}
		glm::vec3 forward = glm::rotate(glm::mat4(1.0f), -angles.x, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), -angles.y, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
		if (inputs.KeyState(inputs.GetCharKey('W')))
		{
			position += forward * 0.005f;
		}
		if (inputs.KeyState(inputs.GetCharKey('S')))
		{
			position -= forward * 0.005f;
		}
		if (inputs.KeyState(inputs.GetCharKey('D')))
		{
			position += right * 0.005f;
		}
		if (inputs.KeyState(inputs.GetCharKey('A')))
		{
			position -= right * 0.005f;
		}
		{
			cam.view = glm::lookAt(position, position + forward, glm::vec3(0.0f, 1.0f, 0.0f));
			cam.viewInverse = glm::inverse(cam.view);
			memcpy(camera_buffer->GetMappedPointer(), &cam, sizeof(Camera));
		}


		execution->StartCommand();
		if (toggle)
		{
			execution->ExecuteRenderPassInstance(render_pass_instance);
		}
		else
		{
			execution->BindRayTracer(raytracer);
			execution->BindRayTracingDescriptorSet(rtset, 0);
			execution->StartRayTracing(shader_table, 0, 2, 1, 1024, 720);
		}
		execution->EndCommand();

		present->StartCommand();
		if (toggle)
		{
			present->CopyToSwapchain_Dynamic(test_color, &new_window);
		}
		else
		{
			present->CopyToSwapchain_Dynamic(test_rtimg, &new_window);
		}
		present->EndCommand();

		MainDevice->ExecuteBatches({ "Main Render", "Present" });

		MainDevice->PresentWindow(new_window);
		new_window.UpdateWindow();
	}
	return 0;
}