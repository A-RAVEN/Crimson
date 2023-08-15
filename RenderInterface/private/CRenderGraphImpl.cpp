#include "pch.h"
#include "CRenderGraphImpl.h"



TextureHandle CRenderGraph_Impl::NewTextureHandle(GPUTextureDescriptor const& textureDesc)
{
	return NewTextureHandle_Internal(textureDesc, nullptr);
}

TextureHandle CRenderGraph_Impl::RegisterWindowBackbuffer(std::shared_ptr<WindowHandle> window)
{
	return NewTextureHandle_Internal(window->GetBackbufferDescriptor(), window);
}

CRenderpassBuilder& CRenderGraph_Impl::NewRenderPass(std::vector<CAttachmentInfo> const& inAttachmentInfo)
{
	return m_RenderPasses.emplace_back(inAttachmentInfo);
}

uint32_t CRenderGraph_Impl::GetRenderNodeCount() const
{
	return m_RenderPasses.size();
}

CRenderpassBuilder const& CRenderGraph_Impl::GetRenderPass(uint32_t nodeID)
{
	return m_RenderPasses[nodeID];
}

TextureHandle CRenderGraph_Impl::TextureHandleByIndex(TIndex index)
{
	auto& handleInternalInfo = m_TextureHandleIdToInternalInfo[index];
	auto& textureDescriptor = m_TextureDescriptorList[handleInternalInfo.m_DescriptorIndex];
	return TextureHandle{ textureDescriptor, index };
}

TextureHandle CRenderGraph_Impl::NewTextureHandle_Internal(GPUTextureDescriptor const& textureDesc, std::shared_ptr<WindowHandle> window)
{
	if (window != nullptr)
	{
		auto found = m_RegisteredTextureHandleIDs.find(window.get());
		if (found != m_RegisteredTextureHandleIDs.end())
		{
			return TextureHandleByIndex(found->second);
		}
	}

	TIndex dataIndex = 0;
	auto found = m_DescriptorToDataID.find(textureDesc);
	if (found == m_DescriptorToDataID.end())
	{
		dataIndex = m_TextureDescriptorList.size();
		m_TextureDescriptorList.push_back(textureDesc);
	}
	else
	{
		dataIndex = found->second;
	}
	TIndex handleIndex = m_TextureHandleIdToInternalInfo.size();
	m_TextureHandleIdToInternalInfo.push_back(TextureHandleInternalInfo{ dataIndex, window });
	m_RegisteredTextureHandleIDs.insert(std::make_pair(static_cast<void*>(window.get()), dataIndex));

	return TextureHandle(textureDesc, handleIndex);
}

