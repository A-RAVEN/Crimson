#pragma once
#include <functional>
#include <string>
#include <vector>

#include "CTextureHandle.h"


class CInlineCommandList
{
public:
	void DrawRendererList();
	void DrawMesh();
	void Dispatch(uint32_t x, uint32_t y, uint32_t z);
};

class CRenderGraphPassConfig
{ 
public:
	void ConfigInputTexture();
	void ConfigOutputColorTarget();
	void ConfigOutputDepthTarget();
	void ConfigPipelineState();
};

class CRenderGraphPassObject
{
public:
	CRenderGraphPassConfig m_Config;
	std::function<void(CInlineCommandList& commandList)> m_Functor;
};

class CRenderGraph
{
public:
	virtual CTextureHandle NewTextureHandle() = 0;

	void NewBufferHandle();

	void ImportTexture();

	void ImportBuffer();

	void PassGlobalShaderValueOverride(std::string const& overrideBufferName);

	void AddPass(CRenderGraphPassConfig const& passConfig, std::function<void(CInlineCommandList& commandList)> const& func)
	{
		CRenderGraphPassObject newPassObject;
		newPassObject.m_Config = passConfig;
		newPassObject.m_Functor = func;
	}
	std::vector<CRenderGraphPassObject> m_GraphPassObjectList;
	std::vector<CTextureMetadata> m_TextureMetadataList;
};