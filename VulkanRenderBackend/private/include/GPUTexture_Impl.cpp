#include "private/include/pch.h"
#include "GPUTexture_Impl.h"


namespace graphics_backend
{
	void GPUTexture_Impl::ScheduleTextureData(uint64_t textureDataOffset, uint64_t dataSize, void* pData)
	{
	}
	void GPUTexture_Impl::DoUpload()
	{
	}
	bool GPUTexture_Impl::UploadingDone() const
	{
		return false;
	}
}