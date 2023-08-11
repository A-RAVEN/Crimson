#pragma once
#include <RenderInterface/header/GPUBuffer.h>
#include "RenderBackendSettings.h"
#include "VulkanApplicationSubobjectBase.h"
#include "CVulkanBufferObject.h"

namespace graphics_backend
{
	class GPUBuffer_Impl : public BaseApplicationSubobject, public GPUBuffer
	{
	public:
		GPUBuffer_Impl(CVulkanApplication& owner);
		virtual void Release() override;
		void Initialize(EBufferUsageFlags usages, uint64_t count, uint64_t stride);
		// 通过 GPUBuffer 继承
		virtual void ScheduleBufferData(uint64_t bufferOffset, uint64_t dataSize, void* pData) override;
		virtual void DoUpload() override;
		virtual bool UploadingDone() const override;
		CVulkanBufferObject const& GetVulkanBufferObject() const { return m_BufferObject; }
	private:
		CVulkanBufferObject m_BufferObject;
		EBufferUsageFlags m_Usages;
		uint64_t m_Count = 0;
		uint64_t m_Stride = 0;
		FrameType m_SubmitFrame = INVALID_FRAMEID;
		std::vector<char> m_ScheduledData;
	};
}

