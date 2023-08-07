#pragma once
#include <SharedTools/header/uhash.h>
#include "Common.h"

struct GPUTextureDescriptor
{
public:
	ETextureFormat format = ETextureFormat::E_R8G8B8A8_UNORM;
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t depth = 1;
	uint32_t mipLevels = 1;
	bool unorderedAccess = false;
	bool operator==(GPUTextureDescriptor const& other) const
	{
		return std::memcmp(this, &other, sizeof(GPUTextureDescriptor)) == 0;
	}
};

template<>
struct hash_utils::is_contiguously_hashable<GPUTextureDescriptor> : public std::true_type {};

class GPUTexture
{
public:
	virtual void ScheduleTextureData(uint64_t textureDataOffset, uint64_t dataSize, void* pData) = 0;
	virtual void DoUpload() = 0;
	virtual bool UploadingDone() const = 0;
protected:
	GPUTextureDescriptor m_Descriptor;
};
