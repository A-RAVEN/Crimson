#pragma once
#include <SharedTools/header/uhash.h>
#include "Common.h"

namespace graphics_backend
{
	struct GPUTextureDescriptor
	{
	public:
		ETextureFormat format = ETextureFormat::E_R8G8B8A8_UNORM;
		ETextureType textureType = ETextureType::e2D;
		ETextureAccessTypeFlags accessType = ETextureAccessType::eSampled;
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t layers = 1;
		uint32_t mipLevels = 1;
		bool operator==(GPUTextureDescriptor const& other) const
		{
			return std::memcmp(this, &other, sizeof(GPUTextureDescriptor)) == 0;
		}
	};

	class GPUTexture
	{
	public:
		virtual void ScheduleTextureData(uint64_t textureDataOffset, uint64_t dataSize, void* pData) = 0;
		virtual void UploadAsync() = 0;
		virtual bool UploadingDone() const = 0;
		virtual GPUTextureDescriptor const& GetDescriptor() const = 0;
	protected:
	};
}


template<>
struct hash_utils::is_contiguously_hashable<graphics_backend::GPUTextureDescriptor> : public std::true_type {};