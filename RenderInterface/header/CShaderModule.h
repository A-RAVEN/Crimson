#pragma once
#include <memory>
#include <string>
#include <SharedTools/header/uhash.h>

#include "HashCommon.h"

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
}

