#pragma once
#include "Common.h"
#include "GPUTexture.h"

class TextureHandle
{
public:
	virtual GPUTextureDescriptor const& GetDescriptor() const {
		return m_Descriptor;
	}
private:
	GPUTextureDescriptor m_Descriptor;
};
