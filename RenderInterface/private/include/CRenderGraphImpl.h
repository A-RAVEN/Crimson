#pragma once
#include <header/CRenderGraph.h>



class CRenderGraph_Impl : public CRenderGraph
{
public:
	virtual TextureHandle NewTextureHandle(GPUTextureDescriptor const& textureDesc) override;
	virtual TextureHandle RegisterWindowBackbuffer(std::shared_ptr<WindowHandle> window) override;
	//virtual TextureHandle RegisterGPUTexture()
	//virtual void SetTextureToBlackboard(std::string const& name, TextureHandle handle) override;
	virtual CRenderpassBuilder& NewRenderPass(std::vector<CAttachmentInfo> const& inAttachmentInfo) override;

	struct TextureHandleInternalInfo
	{
	public:
		TIndex m_DescriptorIndex = INVALID_INDEX;
		std::shared_ptr<WindowHandle> p_WindowsHandle = nullptr;
	};
private:
	TextureHandle NewTextureHandle_Internal(GPUTextureDescriptor const& textureDesc, std::shared_ptr<WindowHandle> window);
private:
	std::deque<CRenderpassBuilder> m_RenderPasses;

	std::vector<GPUTextureDescriptor> m_DescriptorList;
	std::unordered_map<GPUTextureDescriptor, uint32_t, hash_utils::default_hashAlg> m_DescriptorToDataID;
	std::vector<TextureHandleInternalInfo> m_TextureHandleIdToDescriptorID;

	std::unordered_map<std::shared_ptr<WindowHandle>, TIndex> m_WindowToTextureHandleID;
}; 