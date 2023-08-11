#include "private/include/pch.h"
#include "GPUBuffer_Impl.h"
#include "CVulkanApplication.h"
#include "CVulkanMemoryManager.h"
#include "InterfaceTranslator.h"

namespace graphics_backend
{
	GPUBuffer_Impl::GPUBuffer_Impl(CVulkanApplication& owner) : BaseApplicationSubobject(owner)
	{
	}
	void GPUBuffer_Impl::Release()
	{
		GetMemoryManager().ReleaseBuffer(m_BufferObject);
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
	void GPUBuffer_Impl::ScheduleBufferData(uint64_t bufferOffset, uint64_t dataSize, void* pData)
	{
		size_t scheduleSize = bufferOffset + dataSize;
		if (scheduleSize > m_ScheduledData.size())
		{
			m_ScheduledData.resize(scheduleSize);
		}
		memcpy(m_ScheduledData.data(), pData, dataSize);
	}
	void GPUBuffer_Impl::DoUpload()
	{
        auto task = GetVulkanApplication().NewTask()->Name("Upload BufferData Task");
        task->Functor([this]()
            {
                auto& memoryManager = GetVulkanApplication().GetMemoryManager();
                auto& threadContext = GetVulkanApplication().AquireThreadContext();
                auto currentFrame = GetVulkanApplication().GetSubmitCounterContext().GetCurrentFrameID();

                std::atomic_thread_fence(std::memory_order_acquire);

				uint64_t byteSize = std::min(m_Count * m_Stride, m_ScheduledData.size());
                auto tempBuffer = memoryManager.AllocateBuffer(
					EMemoryType::CPU_Sequential_Access
					, EMemoryLifetime::FrameBound
					, byteSize
					, vk::BufferUsageFlagBits::eTransferSrc);
                memcpy(tempBuffer.GetMappedPointer(), m_ScheduledData.data(), byteSize);
                auto cmdBuffer = threadContext.GetCurrentFramePool().AllocateOnetimeCommandBuffer();
                cmdBuffer.copyBuffer(tempBuffer.GetBuffer(), m_BufferObject.GetBuffer(), vk::BufferCopy(0, 0, byteSize));
                cmdBuffer.end();
				memoryManager.ReleaseBuffer(tempBuffer);
                std::atomic_thread_fence(std::memory_order_release);
                m_SubmitFrame = currentFrame;
				GetVulkanApplication().ReturnThreadContext(threadContext);
            });
	}
	bool GPUBuffer_Impl::UploadingDone() const
	{
		if (m_SubmitFrame == INVALID_FRAMEID)
			return false;
		auto& frameContext = GetVulkanApplication().GetSubmitCounterContext();
		if (!frameContext.AnyFrameFinished())
			return false;
		return frameContext.GetReleasedFrameID() >= m_SubmitFrame;
	}
}
