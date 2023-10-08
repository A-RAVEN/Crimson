#pragma once
#include <RenderInterface/header/TextureSampler.h>
#include "Object"

namespace graphics_backend
{
	class TextureSampler_Impl : public TextureSampler, public BaseOb
	{
	public:

	private:
		TextureSamplerDescriptor m_Descriptor;
	};
}


