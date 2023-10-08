#pragma once
#include "Common.h"
#include "GPUTexture.h"

namespace graphics_backend
{
	class TextureHandle
	{
	public:

		TextureHandle(GPUTextureDescriptor const descriptor, TIndex handleIndex) :
			m_Descriptor(descriptor)
			, m_HandleIndex(handleIndex)
		{
		}

		GPUTextureDescriptor const& GetDescriptor() const {
			return m_Descriptor;
		}

		TIndex GetHandleIndex() const { return m_HandleIndex; }
	private:
		GPUTextureDescriptor m_Descriptor;
		TIndex m_HandleIndex;
	};
}