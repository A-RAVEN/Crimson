#pragma once
#include <string>

namespace graphics_backend
{

	class CShaderUniformSet
	{
		
	};

	class CShaderLayoutBuilder
	{
	public:
		CShaderLayoutBuilder& Texture(std::string const& name);
		CShaderLayoutBuilder& ConstantBuffer(std::string const& name);
		CShaderLayoutBuilder& Buffer(std::string const& name);
	};

	static CShaderLayoutBuilder s_Test = CShaderLayoutBuilder{}
		.ConstantBuffer("Test1")
		.Buffer("Test0")
		.Texture("Test2");
}
