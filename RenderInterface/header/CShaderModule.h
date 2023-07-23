#pragma once
#include <memory>
#include <string>
#include <SharedTools/header/uhash.h>

#include "HashCommon.h"

class CShaderModule;

namespace graphics_backend
{
	using namespace hash_utils;
	class ShaderProvider
	{
	public:
		virtual uint64_t GetDataLength(std::string const& codeType) const = 0;
		virtual void* GetDataPtr(std::string const& codeType) const = 0;
		virtual std::string GetUniqueName() const = 0;
		bool operator==(ShaderProvider const& other) const
		{
			return GetUniqueName() == other.GetUniqueName();
		}
		template <class HashAlgorithm>
		friend void hash_append(HashAlgorithm& h, ShaderProvider const& provider) noexcept
		{
			hash_append(h, provider.GetUniqueName());
		}
	};

	class CGraphicsShaderStates
	{
	public:
		std::shared_ptr<ShaderProvider> vertexShader;
		std::shared_ptr<ShaderProvider> fragmentShader;

		bool operator==(CGraphicsShaderStates const& other) const
		{
			return (vertexShader == other.vertexShader
				&& fragmentShader == other.fragmentShader);
		}
		template <class HashAlgorithm>
		friend void hash_append(HashAlgorithm& h, CGraphicsShaderStates const& graphics_states) noexcept
		{
			hash_append(h, graphics_states.vertexShader.get());
			hash_append(h, graphics_states.fragmentShader.get());
		}
	};
}

