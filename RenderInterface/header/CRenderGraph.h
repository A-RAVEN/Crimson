#pragma once
#include <functional>
#include <string>
#include <vector>

#include "CGPUPrimitiveResource.h"
#include "CNativeRenderPassInfo.h"
#include "CTextureHandle.h"


namespace graphics_backend
{
	class CGPUTextureResource;
}



class CRenderGraph
{
public:
	virtual TextureHandle NewTextureHandle(GPUTextureDescriptor const& textureDesc) = 0;
	virtual void SetBlackboard(std::string const& name, TextureHandle handle);
	virtual CRenderpassBuilder BeginRenderPass() = 0;
};