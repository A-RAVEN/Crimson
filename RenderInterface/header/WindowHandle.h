#pragma once
#include <string>
#include "GPUTexture.h"

namespace graphics_backend
{
	struct uint2
	{
		uint32_t x;
		uint32_t y;
	};

	class WindowHandle
	{
	public:
		virtual std::string GetName() const = 0;
		virtual uint2 const& GetSize() = 0;
		virtual GPUTextureDescriptor const& GetBackbufferDescriptor() const = 0;
	};
}