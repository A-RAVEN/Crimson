#pragma once
#include <Generals.h>
#include <Pipeline.h>
#include <GPUDevice.h>
#include <deque>
#include <headers/Debug.h>
#include <headers/LuaInterface/LuaInterfaces.h>

using namespace Crimson;

//class RenderTargetDescriptor
//{
//public:
//	bool b_ResizeWithWindow;
//	float m_WindowResolutionPercentage;
//
//	uint32_t m_Width;
//	uint32_t m_Height;
//
//	EFormat m_ImageFormat;
//	std::vector<EImageUsage> m_Usages;
//
//	std::string m_RenderTargetName;
//};
//
//class FramebufferDescriptor
//{
//public:
//	std::vector<std::string> m_ReferenceImages;
//};
//
//class RenderContext
//{
//
//};



//class GraphicsRenderPipelineInstance;
//class GraphicsRenderPass
//{
//public:
//	PRenderPass m_RenderPass;
//	FramebufferDescriptor m_FrameBufferDescriptor;
//
//	std::vector<PGraphicsPipeline> m_Pipelines;
//	std::vector<RenderTargetDescriptor> m_LocalRenderTargetDescriptors;
//	std::vector<uint32_t> m_ExternalRenderTargetReferences;
//
//	PRenderPassInstance CreateRenderPassInstance()
//	{
//
//	}
//
//	virtual void ConfigureRenderPass() = 0;
//	virtual void ExecuteRenderPass() = 0;
//	//command logic informations
//};
//
//class GraphicsRenderPassInstance
//{
//public:
//	GraphicsRenderPass* p_RenderPass;
//	PFramebuffer m_FrameBuffer;
//	std::map<std::string, PGPUImage> m_Images;
//	//inject render target from pipeline
//	void SetRenderTarget(std::string const& identifier, PGPUImage image)
//	{
//		if (m_Images.find(identifier) != m_Images.end())
//		{
//			LOG_ERR("render target " + identifier + " already exist");
//		}
//		m_Images.insert(std::make_pair(identifier, image));
//	}
//	void BuildInstance(PGPUDevice device)
//	{
//		//create local rendertargets
//		for (RenderTargetDescriptor& descriptor : p_RenderPass->m_LocalRenderTargetDescriptors)
//		{
//			PGPUImage new_img = device->CreateImage(descriptor.m_ImageFormat, descriptor.m_Width, descriptor.m_Height, 1, descriptor.m_Usages, EMemoryType::E_MEMORY_TYPE_DEVICE);
//			m_Images.insert(std::make_pair(descriptor.m_RenderTargetName, new_img));
//		}
//
//		m_FrameBuffer = device->CreateFramebuffer();
//		m_FrameBuffer->m_Images.clear();
//		for (auto& identifier : p_RenderPass->m_FrameBufferDescriptor.m_ReferenceImages)
//		{
//			auto found = m_Images.find(identifier);
//			if (found != m_Images.end())
//			{
//				m_FrameBuffer->m_Images.push_back(found->second);
//			}
//			else
//			{
//				LOG_ERR(std::string("Unfound render target identifier " + identifier + " for render pass"));
//			}
//		}
//	}
//};
//
//
//class GraphicsRenderPipelineInstance
//{
//public:
//	PGPUDevice m_Device;
//	//src render pipeline
//	GraphicsRenderPipeline const* p_RenderPipeline;
//	//frame buffers and images
//	std::vector<PGPUImage> m_FrameBufferImages;
//	//render pass instances
//	std::vector<GraphicsRenderPassInstance> m_RenderPassInstances;
//};
//
//class GraphicsRenderPipeline
//{
//public:
//	//render passes
//	std::deque<GraphicsRenderPass> m_RenderPasses;
//
//	GraphicsRenderPipelineInstance CreateRenderPipelineInstance()
//	{
//		GraphicsRenderPipelineInstance return_val;
//		return_val.p_RenderPipeline = this;
//		for (uint32_t i = 0; i < m_RenderPasses.size(); ++i)
//		{
//			GraphicsRenderPassInstance new_instance;
//			new_instance.p_RenderPass = &m_RenderPasses[i];
//			return_val.m_RenderPassInstances.push_back(new_instance);
//		}
//		return return_val;
//	}
//};

class GraphicsContext
{
public:
	GraphicsContext(PGPUDevice device);
	PGPUDevice m_Device;
	std::vector<PGPUImage> m_RenderTargets;
	std::vector<PFramebuffer> m_FrameBuffers;
	std::vector<PGraphicsPipeline> m_LocalGraphicsPipelines;
	std::vector<PRenderPass> m_RenderPasses;
	std::vector<PRenderPassInstance> m_RenderPassInstances;

	Ptr64 CreateResizableRenderTarget(uint32_t format, bool can_sample, bool can_blit);
	Ptr64 CreateFramebuffer(std::vector<Ptr64> const& renderTargets);
	Ptr64 CreateRenderPassInstance(Ptr64 renderPass, Ptr64 frameBuffer);
	Ptr64 AddRenderPass(PRenderPass renderPass);
	Ptr64 AddLocalGraphicsPipelines(PGraphicsPipeline pipeline);
	void ClearContext();
};