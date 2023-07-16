#pragma once
#include <memory>
#include <string>
namespace graphics_backend
{
	class ShaderProvider
	{
	public:
		virtual uint64_t GetDataLength(std::string const& codeType) const = 0;
		virtual void* GetDataPtr(std::string const& codeType) const = 0;
	};

	class CShaderModule
	{
	public:
		virtual void SetShaderProvider(std::shared_ptr<const ShaderProvider> provider) = 0;
		virtual void Submit() = 0;
		virtual bool Done() = 0;
	};

	class CGraphicsShaderStates
	{
	public:
		std::shared_ptr<CShaderModule> vertexShader;
		std::shared_ptr<CShaderModule> fragmentShader;
	};
}