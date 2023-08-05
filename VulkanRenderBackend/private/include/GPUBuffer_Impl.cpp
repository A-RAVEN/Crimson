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
	void GPUBuffer_Impl::InitializeGPUBuffer(EBufferUsageFlags usages, uint64_t count, uint64_t stride)
	{
		CVulkanMemoryManager& memoryManager = GetVulkanApplication().GetMemoryManager();
		m_BufferObject = memoryManager.AllocateBuffer(
			EMemoryType::GPU
			, EMemoryLifetime::Persistent
			, count * stride
			, EBufferUsageFlagsTranslate(usages));
	}
	void GPUBuffer_Impl::ScheduleBufferData(uint64_t bufferOffset, uint64_t dataSize, void* pData)
	{
	}
	void GPUBuffer_Impl::DoUpload()
	{
	}
	bool GPUBuffer_Impl::Submitting() const
	{
		return false;
	}
}
