#pragma once
#include <string>

namespace graphics_backend
{

	class CShaderLayoutBuilder
	{
	public:
		CShaderLayoutBuilder& Texture(std::string const& name);
		CShaderLayoutBuilder& ConstantBuffer(std::string const& name);
		CShaderLayoutBuilder& Buffer(std::string const& name);
	};
}
