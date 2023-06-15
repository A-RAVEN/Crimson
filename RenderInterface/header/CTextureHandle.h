#pragma once
#include <cstdint>
#include "Common.h"

class CTextureInfo
{
	uint32_t m_Width = 0;
	uint32_t m_Height = 0;
	uint32_t m_Slices = 0;
	uint32_t m_MipCount = 0;
	ETextureFormat m_TextureFormat = ETextureFormat::E_ALL;
};

class CTextureMetadata
{
private:
	CTextureInfo m_TextureInfo;
	bool m_Imported = false;
	bool m_Readonly = false;
};

class CTextureHandle
{
private:
	uint32_t m_MetadataID;
};