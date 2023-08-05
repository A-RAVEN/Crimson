#pragma once
#include <RenderInterface/header/GPUBuffer.h>
#include "VulkanApplicationSubobjectBase.h"

namespace graphics_backend
{
	class GPUBuffer_Impl : public BaseApplicationSubobject, public GPUBuffer
	{
	public:
		GPUBuffer_Impl(CVulkanApplication& owner);
		void InitializeGPUBuffer(EBufferUsageFlags usages, uint64_t count, uint64_t stride);
		// 通过 GPUBuffer 继承
		virtual void ScheduleBufferData(uint64_t bufferOffset, uint64_t dataSize, void* pData) override;
		virtual void DoUpload() override;
		virtual bool Submitting() const override;
	private:
		CVulkanBufferObject m_BufferObject;
	};
}

