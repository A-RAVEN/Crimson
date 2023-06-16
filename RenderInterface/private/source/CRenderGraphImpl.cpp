#include "private/include/pch.h"
#include "private/include/CRenderGraphImpl.h"

CTextureMetadata::CTextureMetadata(CTextureInfo const& textureInfo, bool imported, bool readonly,
	TIndex texturePoolId):
	m_TextureInfo(textureInfo),
	m_Imported(imported),
	m_Readonly(readonly),
	m_ImportedTextureID(texturePoolId)
{
}

void CRenderGraph_Impl::AddPass(CRenderGraphPassConfig const& passConfig, std::function<void(CInlineCommandList& commandList)> const& func)
{
	CRenderGraphPassObject newPassObject;
	newPassObject.m_Config = passConfig;
	newPassObject.m_Functor = func;
}

CTextureHandle CRenderGraph_Impl::NewTextureHandle(CTextureInfo const& textureInfo)
{
	CTextureMetadata const newData(textureInfo);
	m_TextureMetadataList.push_back(newData);
	CTextureHandle result;
	result.m_MetadataID = static_cast<TIndex>(m_TextureMetadataList.size()) - 1u;
	return result;
}
