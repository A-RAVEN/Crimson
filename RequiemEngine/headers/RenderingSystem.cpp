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
			m_ExecutionCmd->BindRayTracer(m_RayTracer);
			m_ExecutionCmd->BindRayTracingDescriptorSet(m_RtSet, 0);
			m_ExecutionCmd->StartRayTracing(m_ShaderTable, 0, 2, 1, 1024, 720);
		//}
		//m_ExecutionCmd->CopyToSwapchain_Dynamic(m_Color, p_Window);
		m_ExecutionCmd->CopyToSwapchain_Dynamic(m_RTColor, p_Window);
		m_ExecutionCmd->EndCommand();
		MainDevice->ExecuteBatches({ "GraphicsLoading", "Main Render" });

		MainDevice->PresentWindow(*p_Window);
		//p_Window->UpdateWindow();
	}
}

RenderingSystem::RenderingSystem(IWindow* window, PAccelerationStructure blas, PAccelerationStructure tlas, PGPUBuffer instance_buffer) : p_Window(window)
{
	PGPUDevice MainDevice = GPUDeviceManager::Get()->GetDevice("MainDevice");


	m_CameraBuffer = MainDevice->CreateBuffer(sizeof(Camera), { EBufferUsage::E_BUFFER_USAGE_UNIFORM }, EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE);


	//memcpy(camera_buffer->GetMappedPointer(), &cam, sizeof(Camera));

	m_Color = MainDevice->CreateImage(EFormat::E_FORMAT_B8G8R8A8_SRGB, 1024, 720, 1, { EImageUsage::E_IMAGE_USAGE_COLOR_ATTACHMENT, EImageUsage::E_IMAGE_USAGE_COPY_SRC }, EMemoryType::E_MEMORY_TYPE_DEVICE);
	m_RTColor = MainDevice->CreateImage(EFormat::E_FORMAT_B8G8R8A8_UNORM, 1024, 720, 1, { EImageUsage::E_IMAGE_USAGE_COPY_SRC, EImageUsage::E_IMAGE_USAGE_STORAGE, EImageUsage::E_IMAGE_USAGE_SAMPLE }, EMemoryType::E_MEMORY_TYPE_DEVICE);
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


	//Try Create Ray Tracing Structure

	m_RtSetLayout = MainDevice->CreateDescriptorSetLayout();
	m_RtSetLayout->m_Bindings.resize(3);
	m_RtSetLayout->m_Bindings[0].m_BindingPoint = 0;
	m_RtSetLayout->m_Bindings[0].m_Num = 1;
	m_RtSetLayout->m_Bindings[0].m_ResourceType = EShaderResourceType::E_SHADER_UNIFORM_BUFFER;
	m_RtSetLayout->m_Bindings[0].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_RAYGEN_NV };
	m_RtSetLayout->m_Bindings[1].m_BindingPoint = 1;
	m_RtSetLayout->m_Bindings[1].m_Num = 1;
	m_RtSetLayout->m_Bindings[1].m_ResourceType = EShaderResourceType::E_SHADER_ACCEL_STRUCT_NV;
	m_RtSetLayout->m_Bindings[1].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_RAYGEN_NV, EShaderType::E_SHADER_TYPE_MISS_NV, EShaderType::E_SHADER_TYPE_CLOSEHIT_NV };
	m_RtSetLayout->m_Bindings[2].m_BindingPoint = 2;
	m_RtSetLayout->m_Bindings[2].m_Num = 1;
	m_RtSetLayout->m_Bindings[2].m_ResourceType = EShaderResourceType::E_SHADER_STORAGE_IMAGE;
	m_RtSetLayout->m_Bindings[2].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_RAYGEN_NV };
	m_RtSetLayout->BuildLayout();

	m_RtSet = m_RtSetLayout->AllocDescriptorSet();
	m_RtSet->WriteDescriptorSetBuffers(0, { m_CameraBuffer }, { {0, sizeof(glm::mat4)} }, 0);
	m_RtSet->WriteDescriptorSetAccelStructuresNV(1, { tlas });
	m_RtSet->WriteDescriptorSetImage(2, m_RTColor, EFilterMode::E_FILTER_MODE_NEAREST, EAddrMode::E_ADDR_MIRRORED_REPEAT, EViewAsType::E_VIEW_AS_COLOR);
	m_RtSet->EndWriteDescriptorSet();

	m_RayTracer = MainDevice->CreateRayTracer();
	{
		auto results = processor.MultiCompile("raytracing.shaders");
		for (auto& itr : results)
		{
			std::cout << static_cast<int>(itr.first) << std::endl;
			m_RayTracer->LoadShaderSource(reinterpret_cast<char*>(itr.second.data()), itr.second.size() * sizeof(uint32_t), itr.first, "default");
		}
	}
	m_RayTracer->m_DescriptorSetLayouts.push_back({ 0, m_RtSetLayout });
	m_RayTracer->m_MaxRecursionDepth = 5;
	m_RayTracer->Build();
	m_ShaderTable = MainDevice->CreateBuffer(m_RayTracer->GetShaderTableSize("default"), { EBufferUsage::E_BUFFER_USAGE_RAYTRACING_NV }, EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE);
	m_RayTracer->CopyShaderTable(m_ShaderTable->GetMappedPointer(), "default");
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
	//	m_RayTracer->LoadShaderSource(reinterpret_cast<char*>(binary.data()), binary.size() * sizeof(uint32_t), EShaderType::E_SHADER_TYPE_RAYGEN_NV, "default");
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
	//	m_RayTracer->LoadShaderSource(reinterpret_cast<char*>(binary.data()), binary.size() * sizeof(uint32_t), EShaderType::E_SHADER_TYPE_CLOSEHIT_NV, "default");
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
	//	m_RayTracer->LoadShaderSource(reinterpret_cast<char*>(binary.data()), binary.size() * sizeof(uint32_t), EShaderType::E_SHADER_TYPE_MISS_NV, "default");
	//}



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

	m_LoadingBuffer = m_RenderingThread->CreateExecutionCommandBuffer(EExecutionCommandType::E_COMMAND_TYPE_GRAPHICS);
	m_LoadingBuffer->StartCommand();
	m_LoadingBuffer->CopyBufferToImage(image_load_buffer, test_loaded_image);
	m_LoadingBuffer->BuildAccelerationStructure(blas);
	m_LoadingBuffer->DeviceMemoryBarrier(EMemoryBarrierType::E_ACCEL_STRUCTURE_BUILD_READ_WRITE);
	m_LoadingBuffer->BuildAccelerationStructure(tlas, instance_buffer);
	m_LoadingBuffer->DeviceMemoryBarrier(EMemoryBarrierType::E_ACCEL_STRUCTURE_BUILD_READ_WRITE);
	m_LoadingBuffer->EndCommand();
	m_RenderingThread->BindExecutionCommandBufferToBatch("GraphicsLoading", m_LoadingBuffer, true);


}

void RenderingSystem::PushBackNewFrame(GraphicsFrame &frame)
{
	m_FrameQueue.try_enqueue(std::move(frame));
	//LockGuard guard(m_FrameQueueLock);
	//m_FrameQueue.push_back(frame);
}

bool RenderingSystem::TryPopFrame(GraphicsFrame& frame)
{
	return m_FrameQueue.try_dequeue(frame);
	//if (!m_FrameQueue.empty())
	//{
	//	LockGuard guard(m_FrameQueueLock);
	//	frame = m_FrameQueue.back();
	//	m_FrameQueue.clear();
	//	return true;
	//}
	//return false;
}

void RenderingSubThread::Work(ThreadWorker const* this_worker)
{
}

void RenderingSubThread::Init(RenderingSystem const* rendering_system, PGPUDevice device)
{
	p_RenderingSystem = rendering_system;
	m_Device = device;
	m_DeviceThreadHandle = m_Device->CreateThread();
}

void RenderingSubThread::PushInstanceQueue(MeshResource* resource, InstanceInfo const& info)
{
	auto find = m_Instances.find(resource);
	if (find == m_Instances.end())
	{
		m_Instances.insert(std::make_pair(resource, MeshInstanceQueue{}));
		find = m_Instances.find(resource);
	}
	find->second.PushInstance(info);
}