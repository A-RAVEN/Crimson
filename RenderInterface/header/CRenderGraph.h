#pragma once
#include <functional>
#include <string>
#include <vector>

#include "CGPUPrimitiveResource.h"
#include "CNativeRenderPassInfo.h"
#include "CTextureHandle.h"
#include "WindowHandle.h"


namespace graphics_backend
{
	class CGPUTextureResource;
}



class CRenderGraph
{
public:
	virtual TextureHandle NewTextureHandle(GPUTextureDescriptor const& textureDesc) = 0;
	virtual TextureHandle RegisterWindowBackbuffer(std::shared_ptr<WindowHandle> window) = 0;
	virtual void SetTextureToBlackboard(std::string const& name, TextureHandle handle) = 0;
	virtual CRenderpassBuilder BeginRenderPass() = 0;
};