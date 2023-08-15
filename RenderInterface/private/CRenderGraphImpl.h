#pragma once
#include <header/CRenderGraph.h>
#include <unordered_set>

class CRenderGraph_Impl : public CRenderGraph
{
public:
	virtual TextureHandle NewTextureHandle(GPUTextureDescriptor const& textureDesc) override;
	virtual TextureHandle RegisterWindowBackbuffer(std::shared_ptr<WindowHandle> window) override;
	virtual CRenderpassBuilder& NewRenderPass(std::vector<CAttachmentInfo> const& inAttachmentInfo) override;

	virtual uint32_t GetRenderNodeCount() const override;
	virtual CRenderpassBuilder const& GetRenderPass(uint32_t nodeID) override;

	struct TextureHandleInternalInfo
	{
	public:
		TIndex m_DescriptorIndex = INVALID_INDEX;
		std::shared_ptr<WindowHandle> p_WindowsHandle = nullptr;
	};
private:
	TextureHandle NewTextureHandle_Internal(GPUTextureDescriptor const& textureDesc, std::shared_ptr<WindowHandle> window);
	TextureHandle TextureHandleByIndex(TIndex index);
private:
	std::deque<CRenderpassBuilder> m_RenderPasses;

	std::vector<GPUTextureDescriptor> m_TextureDescriptorList;
	std::unordered_map<GPUTextureDescriptor, uint32_t, hash_utils::default_hashAlg> m_DescriptorToDataID;
	std::vector<TextureHandleInternalInfo> m_TextureHandleIdToInternalInfo;
	std::unordered_map<void*, TIndex> m_RegisteredTextureHandleIDs;

	std::unordered_map<std::shared_ptr<WindowHandle>, TIndex> m_WindowToTextureHandleID;
}; 