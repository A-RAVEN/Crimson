#include "RenderingSystem.h"
#include <headers/Win32Window.h>
#include <Compiler.h>
#include <fstream>
#include <iostream>
#include <array>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <headers/MeshResource.h>
#include <GPUDevice.h>
#include <headers/ShaderProcessor.h>
#include <headers/VertexData.h>
#include <headers/stb_image.h>

void RenderingSystem::Work(ThreadWorker const* this_worker)
{
	while (this_worker->Working())
	{
		PGPUDevice MainDevice = GPUDeviceManager::Get()->GetDevice("MainDevice");
		//update frame
		{
			GraphicsFrame new_frame{};
			if (TryPopFrame(new_frame))
			{
				for (auto& list : m_Instances)
				{
					list.second.Clear();
				}
				for (auto& inst : new_frame.m_InstanceList)
				{
					auto find = m_Instances.find(inst.first);
					if (find == m_Instances.end())
					{
						m_Instances.insert(std::make_pair(inst.first, MeshInstanceQueue{}));
						find = m_Instances.find(inst.first);
						find->second.m_Resource = inst.first;
					}
					for (auto& itr : inst.second)
					{
						find->second.PushInstance(itr);
						m_TransformManager.ExtendBufferPages(itr.m_BatchId);
					}
				}
				for (auto& update : new_frame.m_TransoformUpdateInfo)
				{
					m_TransformManager.GetData(update.m_BatchId, update.m_TransformId)->m_ModelTransform = update.m_Matrix;
				}
				memcpy(m_CameraBuffer->GetMappedPointer(), &new_frame.m_Camera, sizeof(Camera));
			}
		}

		PGraphicsCommandBuffer cmd = m_RenderingThread->StartSubpassCommand(m_RenderPassInstance, 0);
		cmd->ViewPort(0.0f, 0.0f, 1024.0f, 720.0f);
		cmd->Sissor(0, 0, 1024, 720);
		cmd->BindSubpassPipeline(m_Pipeline);
		cmd->BindSubpassDescriptorSets({ m_Set });
		for (uint32_t i = 0; i < m_TransformManager.GetBatchCount(); ++i)
		{
			cmd->BindSubpassDescriptorSets({ m_TransformManager.GetSet(i) }, 1);
			for (auto& pair : m_Instances)
			{
				pair.second.CmdDrawInstances(cmd, i);
			}
		}
		cmd->EndCommandBuffer();


		m_ExecutionCmd->StartCommand();
		//m_ExecutionCmd->BuildAccelerationStructure(tlas, geometry_instance_buffer, 0, true);
		//m_ExecutionCmd->DeviceMemoryBarrier(EMemoryBarrierType::E_ACCEL_STRUCTURE_BUILD_READ_WRITE);
		m_ExecutionCmd->ExecuteRenderPassInstance(m_RenderPassInstance);
		//if (toggle)
		//{
		//}
		//else
		//{
		//	execution->BindRayTracer(raytracer);
		//	execution->BindRayTracingDescriptorSet(rtset, 0);
		//	execution->StartRayTracing(shader_table, 0, 2, 1, 1024, 720);
		//}
		m_ExecutionCmd->CopyToSwapchain_Dynamic(m_Color, p_Window);
		m_ExecutionCmd->EndCommand();
		MainDevice->ExecuteBatches({ "Main Render" });

		MainDevice->PresentWindow(*p_Window);
		//p_Window->UpdateWindow();
	}
}

RenderingSystem::RenderingSystem(IWindow* window) : p_Window(window)
{
	PGPUDevice MainDevice = GPUDeviceManager::Get()->GetDevice("MainDevice");


	//Try Create Ray Tracing Structure
	/*PRayTracer raytracer = MainDevice->CreateRayTracer();
	PRayTraceGeometry raytrace_geometry = MainDevice->CreateRayTraceGeometry();
	PAccelerationStructure accel_struct = MainDevice->CreateAccelerationStructure();
	raytrace_geometry->SetVertexData(new_resource.m_VertexBuffer, 0, new_resource.m_VertexSize, sizeof(VertexDataLightWeight), EDataType::EVEC3);
	raytrace_geometry->SetIndexData(new_resource.m_IndexBuffer, 0, new_resource.m_IndexSize, EIndexType::E_INDEX_TYPE_32);
	raytrace_geometry->SetGeometryFlags({ EGeometryFlags::E_GEOMETRY_OPAQUE });
	raytrace_geometry->SetGeometryType(ERayTraceGeometryType::E_GEOMETRY_TYPE_TRIANGLES);
	accel_struct->m_Geometries = { raytrace_geometry };
	accel_struct->m_BuildFlags = { EBuildAccelerationStructureFlags::E_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV };
	accel_struct->InitAS();
	accel_struct->SetupScratchBuffer();

	PAccelerationStructure tlas = MainDevice->CreateAccelerationStructure();
	tlas->m_InstanceNumber = 10;
	tlas->m_BuildFlags = { EBuildAccelerationStructureFlags::E_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV, EBuildAccelerationStructureFlags::E_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV };
	tlas->InitAS(true);
	tlas->SetupScratchBuffer();*/

	m_CameraBuffer = MainDevice->CreateBuffer(sizeof(Camera), { EBufferUsage::E_BUFFER_USAGE_UNIFORM }, EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE);
	//PGPUBuffer geometry_instance_buffer = MainDevice->CreateBuffer(sizeof(RayTraceGeometryInstance) * 5, { EBufferUsage::E_BUFFER_USAGE_RAYTRACING_NV }, EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE);
	//RayTraceGeometryInstance* pGeometryInstance = new (geometry_instance_buffer->GetMappedPointer()) RayTraceGeometryInstance[5];
	//for (int i = 0; i < 5; ++i)
	//{
	//	pGeometryInstance[i].m_Flags = static_cast<uint32_t>(EGeometryInstanceFlags::E_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE);
	//	pGeometryInstance[i].m_InstanceId = 0;
	//	pGeometryInstance[i].m_Mask = 0x1;// 0xff;
	//	pGeometryInstance[i].m_AccelerationStructureHandle = accel_struct->GetHandle();
	//	*(reinterpret_cast<glm::mat3x4*>(&pGeometryInstance[i].m_TransformMatrix)) = glm::transpose(glm::translate(glm::mat4(1.0f), glm::vec3(i * 2.0f, 0.0f, 0.0f)));
	//}

	//memcpy(camera_buffer->GetMappedPointer(), &cam, sizeof(Camera));

	m_Color = MainDevice->CreateImage(EFormat::E_FORMAT_B8G8R8A8_SRGB, 1024, 720, 1, { EImageUsage::E_IMAGE_USAGE_COLOR_ATTACHMENT, EImageUsage::E_IMAGE_USAGE_COPY_SRC }, EMemoryType::E_MEMORY_TYPE_DEVICE);
	//PGPUImage test_rtimg = MainDevice->CreateImage(EFormat::E_FORMAT_B8G8R8A8_UNORM, 1024, 720, 1, { EImageUsage::E_IMAGE_USAGE_COPY_SRC, EImageUsage::E_IMAGE_USAGE_STORAGE, EImageUsage::E_IMAGE_USAGE_SAMPLE }, EMemoryType::E_MEMORY_TYPE_DEVICE);
	PGPUImage test_depth_stencil = MainDevice->CreateImage(EFormat::E_FORMAT_D24_UNORM_S8_UINT, 1024, 720, 1, { EImageUsage::E_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT }, EMemoryType::E_MEMORY_TYPE_DEVICE);
	PRenderPass test_renderpass = MainDevice->CreateRenderPass();
	test_renderpass->m_Attachments = { {EFormat::E_FORMAT_B8G8R8A8_SRGB, EAttachmentClearType::E_ATTACHMENT_CLEAR_ZEROS} , { EFormat::E_FORMAT_D24_UNORM_S8_UINT, EAttachmentClearType::E_ATTACHMENT_CLEAR_ONES } };
	test_renderpass->m_Subpasses.resize(1);
	test_renderpass->m_Subpasses[0].m_OutputAttachments = { 0 };
	test_renderpass->m_Subpasses[0].m_DepthStencilAttachment = 1;
	test_renderpass->BuildRenderPass();

	m_SetLayout = MainDevice->CreateDescriptorSetLayout();
	m_SetLayout->m_Bindings.resize(2);
	m_SetLayout->m_Bindings[0].m_BindingPoint = 0;
	m_SetLayout->m_Bindings[0].m_Num = 1;
	m_SetLayout->m_Bindings[0].m_ResourceType = EShaderResourceType::E_SHADER_UNIFORM_BUFFER;
	m_SetLayout->m_Bindings[0].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_VERTEX, EShaderType::E_SHADER_TYPE_FRAGMENT };
	m_SetLayout->m_Bindings[1].m_BindingPoint = 1;
	m_SetLayout->m_Bindings[1].m_Num = 1;
	m_SetLayout->m_Bindings[1].m_ResourceType = EShaderResourceType::E_SHADER_IMAGE_SAMPLER;
	m_SetLayout->m_Bindings[1].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_FRAGMENT };

	//PDescriptorSetLayout rtlayout = MainDevice->CreateDescriptorSetLayout();
	//rtlayout->m_Bindings.resize(3);
	//rtlayout->m_Bindings[0].m_BindingPoint = 0;
	//rtlayout->m_Bindings[0].m_Num = 1;
	//rtlayout->m_Bindings[0].m_ResourceType = EShaderResourceType::E_SHADER_UNIFORM_BUFFER;
	//rtlayout->m_Bindings[0].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_RAYGEN_NV };

	//rtlayout->m_Bindings[1].m_BindingPoint = 1;
	//rtlayout->m_Bindings[1].m_Num = 1;
	//rtlayout->m_Bindings[1].m_ResourceType = EShaderResourceType::E_SHADER_ACCEL_STRUCT_NV;
	//rtlayout->m_Bindings[1].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_RAYGEN_NV, EShaderType::E_SHADER_TYPE_MISS_NV, EShaderType::E_SHADER_TYPE_CLOSEHIT_NV };

	//rtlayout->m_Bindings[2].m_BindingPoint = 2;
	//rtlayout->m_Bindings[2].m_Num = 1;
	//rtlayout->m_Bindings[2].m_ResourceType = EShaderResourceType::E_SHADER_STORAGE_IMAGE;
	//rtlayout->m_Bindings[2].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_RAYGEN_NV };
	//rtlayout->BuildLayout();

	m_Set = m_SetLayout->AllocDescriptorSet();
	m_Set->WriteDescriptorSetBuffers(0, { m_CameraBuffer }, { {0, sizeof(glm::mat4)} }, 0);
	m_Set->EndWriteDescriptorSet();

	m_Pipeline = MainDevice->CreateGraphicsPipeline();

	ShaderProcessor processor;
	{
		auto results = processor.MultiCompile("test.shaders");
		for (auto& itr : results)
		{
			std::cout << static_cast<int>(itr.first) << std::endl;
			m_Pipeline->LoadShaderSource(reinterpret_cast<char*>(itr.second.data()), itr.second.size() * sizeof(uint32_t), itr.first);
		}
	}
	//{
	//	auto results = processor.MultiCompile("raytracing.shaders");
	//	for (auto& itr : results)
	//	{
	//		std::cout << static_cast<int>(itr.first) << std::endl;
	//		raytracer->LoadShaderSource(reinterpret_cast<char*>(itr.second.data()), itr.second.size() * sizeof(uint32_t), itr.first, "default");
	//	}
	//}

	m_Pipeline->m_VertexInputs.resize(2);
	m_Pipeline->m_VertexInputs[0].m_DataTypes = VertexDataLightWeight::GetDataType();
	m_Pipeline->m_VertexInputs[0].m_VertexInputMode = EVertexInputMode::E_VERTEX_INPUT_PER_VERTEX;
	m_Pipeline->m_VertexInputs[1].m_DataTypes = { EDataType::EUINT };
	m_Pipeline->m_VertexInputs[1].m_VertexInputMode = EVertexInputMode::E_VERTEX_INPUT_PER_INSTANCE;
	m_Pipeline->m_DepthRule = EDepthTestRule::E_DEPTH_TEST_ENABLED;
	m_Pipeline->m_StencilRule = EStencilRule::E_STENCIL_WRITE;
	m_Pipeline->m_DescriptorSetLayouts.push_back({ 0, m_SetLayout });
	m_Pipeline->m_DescriptorSetLayouts.push_back({ 1, m_TransformManager.GetSetLayout() });
	test_renderpass->InstanciatePipeline(m_Pipeline, 0);

	//{
	//	std::ifstream shader_src("raytrace.rgen");
	//	std::string src;
	//	if (shader_src.is_open())
	//	{
	//		std::string line;
	//		while (std::getline(shader_src, line))
	//		{
	//			src += line + "\n";
	//		}
	//	}
	//	src = compiler->PreprocessGLSLShader("raytrace.rgen", src, ShaderCompiler::ECompileShaderType::E_SHADER_TYPE_RAYGEN_NV);
	//	std::cout << src << std::endl;
	//	auto binary = compiler->CompileGLSLShaderSource("raytrace.rgen", src, ShaderCompiler::ECompileShaderType::E_SHADER_TYPE_RAYGEN_NV);
	//	raytracer->LoadShaderSource(reinterpret_cast<char*>(binary.data()), binary.size() * sizeof(uint32_t), EShaderType::E_SHADER_TYPE_RAYGEN_NV, "default");
	//}

	//{
	//	std::ifstream shader_src("closehit.rchit");
	//	std::string src;
	//	if (shader_src.is_open())
	//	{
	//		std::string line;
	//		while (std::getline(shader_src, line))
	//		{
	//			src += line + "\n";
	//		}
	//	}
	//	src = compiler->PreprocessGLSLShader("closehit.rchit", src, ShaderCompiler::ECompileShaderType::E_SHADER_TYPE_CLOSEHIT_NV);
	//	std::cout << src << std::endl;
	//	auto binary = compiler->CompileGLSLShaderSource("closehit.rchit", src, ShaderCompiler::ECompileShaderType::E_SHADER_TYPE_CLOSEHIT_NV);
	//	raytracer->LoadShaderSource(reinterpret_cast<char*>(binary.data()), binary.size() * sizeof(uint32_t), EShaderType::E_SHADER_TYPE_CLOSEHIT_NV, "default");
	//}
	//{
	//	std::ifstream shader_src("miss.rmiss");
	//	std::string src;
	//	if (shader_src.is_open())
	//	{
	//		std::string line;
	//		while (std::getline(shader_src, line))
	//		{
	//			src += line + "\n";
	//		}
	//	}
	//	src = compiler->PreprocessGLSLShader("miss.rmiss", src, ShaderCompiler::ECompileShaderType::E_SHADER_TYPE_MISS_NV);
	//	std::cout << src << std::endl;
	//	auto binary = compiler->CompileGLSLShaderSource("miss.rmiss", src, ShaderCompiler::ECompileShaderType::E_SHADER_TYPE_MISS_NV);
	//	raytracer->LoadShaderSource(reinterpret_cast<char*>(binary.data()), binary.size() * sizeof(uint32_t), EShaderType::E_SHADER_TYPE_MISS_NV, "default");
	//}
	//raytracer->m_DescriptorSetLayouts.push_back({ 0, rtlayout });
	//raytracer->m_MaxRecursionDepth = 5;
	//raytracer->Build();
	//PGPUBuffer shader_table = MainDevice->CreateBuffer(raytracer->GetShaderTableSize("default"), { EBufferUsage::E_BUFFER_USAGE_RAYTRACING_NV }, EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE);
	//raytracer->CopyShaderTable(shader_table->GetMappedPointer(), "default");

	PFramebuffer test_framebuffer = MainDevice->CreateFramebuffer();
	test_framebuffer->m_Images = { m_Color, test_depth_stencil };

	m_RenderPassInstance = MainDevice->CreateRenderPassInstance(test_renderpass, test_framebuffer);

	MainDevice->CreateBatch("GraphicsLoading", EExecutionCommandType::E_COMMAND_TYPE_GRAPHICS, 0);
	MainDevice->CreateBatch("Main Render", EExecutionCommandType::E_COMMAND_TYPE_GRAPHICS, 0);
	//MainDevice->CreateBatch("Present", EExecutionCommandType::E_COMMAND_TYPE_GRAPHICS, 0);

	m_RenderingThread = MainDevice->CreateThread();

	m_ExecutionCmd = m_RenderingThread->CreateExecutionCommandBuffer(EExecutionCommandType::E_COMMAND_TYPE_GRAPHICS);
	m_RenderingThread->BindExecutionCommandBufferToBatch("Main Render", m_ExecutionCmd);

	//PExecutionCommandBuffer present = m_RenderingThread->CreateExecutionCommandBuffer(EExecutionCommandType::E_COMMAND_TYPE_GRAPHICS);
	//m_RenderingThread->BindExecutionCommandBufferToBatch("Present", present);

	PGPUBuffer image_load_buffer;
	PGPUImage test_loaded_image;
	{
		int w, h, channel_num;
		auto data = stbi_load("testnormal1.jpg", &w, &h, &channel_num, 4);
		image_load_buffer = MainDevice->CreateBuffer(w * h * sizeof(int), { EBufferUsage::E_BUFFER_USAGE_COPY_SRC }, EMemoryType::E_MEMORY_TYPE_HOST);
		memcpy(image_load_buffer->GetMappedPointer(), data, w * h * sizeof(int));
		stbi_image_free(data);
		image_load_buffer->UnMapp();
		test_loaded_image = MainDevice->CreateImage(EFormat::E_FORMAT_R8G8B8A8_UNORM, w, h, 1, { EImageUsage::E_IMAGE_USAGE_COPY_DST, EImageUsage::E_IMAGE_USAGE_SAMPLE }
		, EMemoryType::E_MEMORY_TYPE_DEVICE);
		m_Set->WriteDescriptorSetImage(1, test_loaded_image, EFilterMode::E_FILTER_MODE_LINEAR, EAddrMode::E_ADDR_MIRRORED_REPEAT);
		m_Set->EndWriteDescriptorSet();
	}

	PExecutionCommandBuffer load = m_RenderingThread->CreateExecutionCommandBuffer(EExecutionCommandType::E_COMMAND_TYPE_GRAPHICS);
	load->StartCommand();
	load->CopyBufferToImage(image_load_buffer, test_loaded_image);
	//load->BuildAccelerationStructure(accel_struct);
	//load->DeviceMemoryBarrier(EMemoryBarrierType::E_ACCEL_STRUCTURE_BUILD_READ_WRITE);
	//load->BuildAccelerationStructure(tlas, geometry_instance_buffer);
	//load->DeviceMemoryBarrier(EMemoryBarrierType::E_ACCEL_STRUCTURE_BUILD_READ_WRITE);
	load->EndCommand();
	m_RenderingThread->BindExecutionCommandBufferToBatch("GraphicsLoading", load, true);

	//PDescriptorSet rtset = rtlayout->AllocDescriptorSet();
	//rtset->WriteDescriptorSetBuffers(0, { camera_buffer }, { {0, sizeof(glm::mat4)} }, 0);
	//rtset->WriteDescriptorSetAccelStructuresNV(1, { tlas });
	//rtset->WriteDescriptorSetImage(2, test_rtimg, EFilterMode::E_FILTER_MODE_NEAREST, EAddrMode::E_ADDR_MIRRORED_REPEAT, EViewAsType::E_VIEW_AS_COLOR);
	//rtset->EndWriteDescriptorSet();
}

void RenderingSystem::PushBackNewFrame(GraphicsFrame const& frame)
{
	LockGuard guard(m_FrameQueueLock);
	m_FrameQueue.push_back(frame);
}

bool RenderingSystem::TryPopFrame(GraphicsFrame& frame)
{
	LockGuard guard(m_FrameQueueLock);
	if (!m_FrameQueue.empty())
	{
		frame = m_FrameQueue.back();
		m_FrameQueue.clear();
		return true;
	}
	return false;
}
