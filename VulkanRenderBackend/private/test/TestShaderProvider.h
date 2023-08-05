#pragma once
#include <RenderInterface/header/ShaderProvider.h>
#include <unordered_map>

namespace graphics_backend
{
	class ShaderProvider_Impl : public ShaderProvider
	{
	public:
		// 通过 ShaderProvider 继承
		virtual uint64_t GetDataLength(std::string const& codeType) const override;
		virtual void const* GetDataPtr(std::string const& codeType) const override;
		virtual std::string GetUniqueName() const override;

		virtual ShaderProvider::ShaderSourceInfo const& GetDataInfo(std::string const& codeType) const override;

		void SetUniqueName(std::string const& uniqueName);

		void SetData(std::string const& codeType, std::string const& entryPoint, void* dataPtr, uint64_t dataLength);

	private:
		std::string m_UniqueName;
		std::unordered_map<std::string
			, std::pair<std::string, std::vector<char>>
		> m_Data;
	};
}