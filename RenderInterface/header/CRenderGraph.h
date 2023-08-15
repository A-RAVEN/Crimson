#pragma once
#include <functional>
#include <string>
#include <vector>

#include "CGPUPrimitiveResource.h"
#include "CNativeRenderPassInfo.h"
#include "CTextureHandle.h"
#include "WindowHandle.h"

class CRenderGraph
{
public:
	//Used By Client
	virtual TextureHandle NewTextureHandle(GPUTextureDescriptor const& textureDesc) = 0;
	virtual TextureHandle RegisterWindowBackbuffer(std::shared_ptr<WindowHandle> window) = 0;
	virtual CRenderpassBuilder& NewRenderPass(std::vector<CAttachmentInfo> const& inAttachmentInfo) = 0;
	//Used By Backend
	virtual uint32_t GetRenderNodeCount() const = 0;
	virtual CRenderpassBuilder const& GetRenderPass(uint32_t nodeID) = 0;
};