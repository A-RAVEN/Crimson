#pragma once
#include <RenderInterface/header/CShaderModule.h>
#include <unordered_map>

namespace graphics_backend
{
	class ShaderProvider_Impl : public ShaderProvider
	{
	public:
		// Í¨¹ý ShaderProvider ¼Ì³Ð
		virtual uint64_t GetDataLength(std::string const& codeType) const override;
		virtual void const* GetDataPtr(std::string const& codeType) const override;
		virtual std::string GetUniqueName() const override;

		void SetUniqueName(std::string const& uniqueName);

		void SetData(std::string const& codeType, void* dataPtr, uint64_t dataLength);

	private:
		std::string m_UniqueName;
		std::unordered_map<std::string, std::vector<char>> m_Data;
	};
}