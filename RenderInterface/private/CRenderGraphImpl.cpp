#include "pch.h"
#include "CRenderGraphImpl.h"


namespace graphics_backend
{

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

	void CRenderGraph_Impl::PresentWindow(std::shared_ptr<WindowHandle> window)
	{
		m_TargetWindow = window;
	}

	ShaderBindingSetHandle CRenderGraph_Impl::NewShaderBindingSetHandle(ShaderBindingBuilder const& builder)
	{
		auto found = m_ShaderBindingDescToIndex.find(builder);
		TIndex foundIndex = INVALID_INDEX;
		if (found == m_ShaderBindingDescToIndex.end())
		{
			foundIndex = m_ShaderBindingDescToIndex.size();
			m_ShaderBindingDescToIndex.insert(std::make_pair(builder, foundIndex));
		}
		else
		{
			foundIndex = found->second;
		}

		return ShaderBindingSetHandle(ShaderBindingSetHandle_Impl{ foundIndex }, nullptr);
	}

	uint32_t CRenderGraph_Impl::GetRenderNodeCount() const
	{
		return m_RenderPasses.size();
	}

	CRenderpassBuilder const& CRenderGraph_Impl::GetRenderPass(uint32_t nodeID) const
	{
		return m_RenderPasses[nodeID];
	}

	TextureHandle CRenderGraph_Impl::TextureHandleByIndex(TIndex index) const
	{
		auto& handleInternalInfo = m_TextureHandleIdToInternalInfo[index];
		auto& textureDescriptor = m_TextureDescriptorList[handleInternalInfo.m_DescriptorIndex];
		return TextureHandle{ textureDescriptor, index };
	}

	TIndex CRenderGraph_Impl::WindowHandleToTextureIndex(std::shared_ptr<WindowHandle> handle) const
	{
		auto found = m_RegisteredTextureHandleIDs.find(handle.get());
		if (found != m_RegisteredTextureHandleIDs.end())
		{
			return found->second;
		}
		return INVALID_INDEX;
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
}