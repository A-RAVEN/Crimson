#pragma once
#include "VulkanApplicationSubobjectBase.h"
#include <RenderInterface/header/GPUTexture.h>

namespace graphics_backend
{
	class GPUTexture_Impl : public BaseApplicationSubobject, public GPUTexture
	{
	public:
		virtual void ScheduleTextureData(uint64_t textureDataOffset, uint64_t dataSize, void* pData) override;
		virtual void DoUpload() override;
		virtual bool UploadingDone() const override;
	};
}

