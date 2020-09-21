#include <headers/Graphics/GraphicsRenderPipeline.h>

using namespace Crimson;
GraphicsContext::GraphicsContext(PGPUDevice device) : m_Device(device)
{
}
Ptr64 GraphicsContext::CreateResizableRenderTarget(uint32_t format, bool can_sample, bool can_blit)
{
	std::vector<Crimson::EImageUsage> usages;
	Crimson::EFormat eformat = static_cast<Crimson::EFormat>(format);
	if (Crimson::IsColorFormat(eformat))
	{
		usages.push_back(EImageUsage::E_IMAGE_USAGE_COLOR_ATTACHMENT);
	}
	else
	{
		usages.push_back(EImageUsage::E_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT);
	}
	if (can_sample)
	{
		usages.push_back(EImageUsage::E_IMAGE_USAGE_SAMPLE);
	}
	if (can_blit)
	{
		usages.push_back(EImageUsage::E_IMAGE_USAGE_COPY_SRC);
		usages.push_back(EImageUsage::E_IMAGE_USAGE_COPY_DST);
	}
	PGPUImage new_rendertarget = m_Device->CreateImage(eformat, 1024, 1024, 1, usages, EMemoryType::E_MEMORY_TYPE_DEVICE);
	m_RenderTargets.push_back(new_rendertarget);
	return Ptr64::WrapUint(m_RenderTargets.size() - 1);
}

Ptr64 GraphicsContext::CreateFramebuffer(std::vector<Ptr64> const& renderTargets)
{
	PFramebuffer framebuffer = m_Device->CreateFramebuffer();
	framebuffer->m_Images.resize(renderTargets.size());
	for (size_t i = 0; i < renderTargets.size(); ++i)
	{
		framebuffer->m_Images[i] = m_RenderTargets[renderTargets[i].uintT];
	}
	m_FrameBuffers.push_back(framebuffer);
	return Ptr64::WrapUint(m_FrameBuffers.size() - 1);
}

Ptr64 GraphicsContext::CreateRenderPassInstance(Ptr64 renderPass, Ptr64 frameBuffer)
{
	PRenderPassInstance instance = m_Device->CreateRenderPassInstance(m_RenderPasses[renderPass.uintT], m_FrameBuffers[frameBuffer.uintT]);
	m_RenderPassInstances.push_back(instance);
	return Ptr64::WrapUint(m_RenderPassInstances.size() - 1);
}

Ptr64 GraphicsContext::AddRenderPass(PRenderPass renderPass)
{
	Ptr64 return_val;
	m_RenderPasses.push_back(renderPass);
	return Ptr64::WrapUint(m_RenderPasses.size() - 1);
}

Ptr64 GraphicsContext::AddLocalGraphicsPipelines(PGraphicsPipeline pipeline)
{
	Ptr64 return_val;
	m_LocalGraphicsPipelines.push_back(pipeline);
	return Ptr64::WrapUint(m_LocalGraphicsPipelines.size() - 1);
}

void GraphicsContext::ClearContext()
{
	for (auto instance : m_RenderPassInstances)
	{
		instance->Dispose();
	}
	for (auto renderpass : m_RenderPasses)
	{
		renderpass->Dispose();
	}
	for (auto framebuffer : m_FrameBuffers)
	{
		framebuffer->Dispose();
	}
	for (auto graphics_pipeline : m_LocalGraphicsPipelines)
	{
		graphics_pipeline->Dispose();
	}
	for (auto rendertarget : m_RenderTargets)
	{
		rendertarget->Dispose();
	}
}
