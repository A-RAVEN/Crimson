#pragma once
#include "Common.h"
#include "GPUTexture.h"

namespace graphics_backend
{
	class CGPUTextureResource
	{
	public:
		virtual GPUTextureDescriptor GetTextureDescriptor() const = 0;
	};
}