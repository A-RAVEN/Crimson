#include "private/include/pch.h"
#include "private/include/CRenderGraphImpl.h"



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

TextureHandle CRenderGraph_Impl::NewTextureHandle_Internal(GPUTextureDescriptor const& textureDesc, std::shared_ptr<WindowHandle> window)
{
	TIndex dataIndex = 0;
	auto found = m_DescriptorToDataID.find(textureDesc);
	if (found == m_DescriptorToDataID.end())
	{
		dataIndex = m_DescriptorList.size();
		m_DescriptorList.push_back(textureDesc);
	}
	else
	{
		dataIndex = found->second;
	}
	TIndex handleIndex = m_TextureHandleIdToDescriptorID.size();
	m_TextureHandleIdToDescriptorID.push_back(TextureHandleInternalInfo{ dataIndex, window });
	return TextureHandle(textureDesc, handleIndex);
}
