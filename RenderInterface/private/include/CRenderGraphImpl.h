#pragma once
#include <header/CRenderGraph.h>

class CRenderGraphPassObject
{
public:
	CRenderGraphPassConfig m_Config;
	std::function<void(CInlineCommandList& commandList)> m_Functor;
};

class CTextureMetadata
{
public:
	CTextureMetadata(CTextureInfo const& textureInfo, bool imported = false, bool readonly = false, TIndex texturePoolId = INVALID_INDEX);
	CTextureInfo m_TextureInfo;
	bool m_Imported = false;
	bool m_Readonly = false;
	TIndex m_ImportedTextureID;
};

class CRenderGraph_Impl : public CRenderGraph
{
public:
	virtual void AddPass(CRenderGraphPassConfig const& passConfig, std::function<void(CInlineCommandList& commandList)> const& func) override;
	virtual CTextureHandle NewTextureHandle(CTextureInfo const& textureInfo) override;
private:
	std::vector<CRenderGraphPassObject> m_GraphPassObjectList;
	std::vector<CTextureMetadata> m_TextureMetadataList;
};