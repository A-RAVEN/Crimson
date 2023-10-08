#include "private/include/pch.h"
#include "GPUBuffer_Impl.h"
#include "CVulkanApplication.h"
#include "CVulkanMemoryManager.h"
#include "InterfaceTranslator.h"

namespace graphics_backend
{
	GPUBuffer_Impl::GPUBuffer_Impl(CVulkanApplication& owner) : BaseUploadingResource(owner)
	{
	}
	void GPUBuffer_Impl::Release()
	{
		m_BufferObject = nullptr;
	}
	void GPUBuffer_Impl::Initialize(EBufferUsageFlags usages, uint64_t count, uint64_t stride)
	{
        m_Usages = usages;
		m_Count = count;
		m_Stride = stride;
		CVulkanMemoryManager& memoryManager = GetVulkanApplication().GetMemoryManager();
		m_BufferObject = memoryManager.AllocateBuffer(
			EMemoryType::GPU
			, EMemoryLifetime::Persistent
			, count * stride
			, EBufferUsageFlagsTranslate(usages) | vk::BufferUsageFlagBits::eTransferDst);
	}
	void GPUBuffer_Impl::UploadAsync()
	{
		BaseUploadingResource::UploadAsync(UploadingResourceType::eMemoryDataLowPriority);
	}
	bool GPUBuffer_Impl::UploadingDone() const
	{
		return BaseUploadingResource::UploadingDone();
	}
	void GPUBuffer_Impl::ScheduleBufferData(uint64_t bufferOffset, uint64_t dataSize, void* pData)
	{
		size_t scheduleSize = bufferOffset + dataSize;
		if (scheduleSize > m_ScheduledData.size())
		{
			m_ScheduledData.resize(scheduleSize);
		}
		memcpy(m_ScheduledData.data() + bufferOffset, pData, dataSize);
	}
	void GPUBuffer_Impl::DoUpload()
	{
        auto& memoryManager = GetMemoryManager();
        auto threadContext = GetVulkanApplication().AquireThreadContextPtr();
        auto currentFrame = GetFrameCountContext().GetCurrentFrameID();

        std::atomic_thread_fence(std::memory_order_acquire);

		uint64_t byteSize = std::min(m_Count * m_Stride, m_ScheduledData.size());
        auto tempBuffer = memoryManager.AllocateFrameBoundTransferStagingBuffer(byteSize);
        memcpy(tempBuffer->GetMappedPointer(), m_ScheduledData.data(), byteSize);
        auto cmdBuffer = threadContext->GetCurrentFramePool().AllocateMiscCommandBuffer("Upload GPU Buffer");
        cmdBuffer.copyBuffer(tempBuffer->GetBuffer(), m_BufferObject->GetBuffer(), vk::BufferCopy(0, 0, byteSize));
        cmdBuffer.end();
		MarkUploadingDoneThisFrame();
	}
}
