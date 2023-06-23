#pragma once
#include "Common.h"

namespace graphics_backend
{
	class CGPUTextureResource
	{
	public:
		virtual CTextureInfo GetTextureInfo() const = 0;
	};
}