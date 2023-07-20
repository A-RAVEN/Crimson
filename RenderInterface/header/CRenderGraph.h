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



class CRenderGraphPassConfig
{ 
public:
	void ConfigInputTexture(CTextureHandle const& );
	void ConfigOutputColorTarget();
	void ConfigOutputDepthTarget();
	void ConfigPipelineState();
};

class CRenderGraph
{
public:
	RENDERINTERFACE_API virtual CTextureHandle NewTextureHandle(CTextureInfo const& textureInfo) = 0;
	RENDERINTERFACE_API virtual void ImportTexture(graphics_backend::CGPUTextureResource const& textureResource) = 0;
	RENDERINTERFACE_API virtual void NewBufferHandle() = 0;
	RENDERINTERFACE_API virtual void ImportBuffer() = 0;
	RENDERINTERFACE_API virtual void PassGlobalShaderValueOverride(std::string const& overrideBufferName) = 0;

	virtual CRenderpassBuilder BeginRenderPass();
};