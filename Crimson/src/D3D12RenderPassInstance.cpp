#include <headers/D3D12RenderPassInstance.h>

namespace Crimson
{
	void D3D12RenderPassInstance::Init(D3D12GPUDevice* owning_device, D3D12RenderPass* renderpass, D3D12Framebuffer* framebuffer)
	{
		p_OwningDevice = owning_device;
		m_SubpassInstances.resize(renderpass->m_D3D12Subpasses.size());
		for (uint32_t i = 0; i < renderpass->m_D3D12Subpasses.size(); ++i)
		{
			auto& subpassInfo = renderpass->m_Subpasses[i];
			auto& subpassSrc = renderpass->m_D3D12Subpasses[i];
			auto& currentSubpass = m_SubpassInstances[i];
			currentSubpass.m_DepthStencilDescriptor = subpassSrc.m_DepthStencilDescriptor;
			currentSubpass.m_RenderTargetDescriptors = subpassSrc.m_RenderTargetDescriptors;
			for (uint32_t outputref_id = 0; outputref_id < subpassInfo.m_OutputAttachments.size(); ++outputref_id)
			{
				currentSubpass.m_RenderTargetDescriptors[outputref_id].cpuDescriptor = framebuffer->m_CPUHandles[subpassInfo.m_OutputAttachments[outputref_id]];
			}
			if (subpassInfo.m_DepthStencilAttachment != -1)
			{
				currentSubpass.m_DepthStencilDescriptor.cpuDescriptor = framebuffer->m_CPUHandles[subpassInfo.m_DepthStencilAttachment];
			}
		}
		p_RenderPass = p_DXRenderPass = renderpass;
		p_Framebuffer = p_DXFramebuffer = framebuffer;
	}
}