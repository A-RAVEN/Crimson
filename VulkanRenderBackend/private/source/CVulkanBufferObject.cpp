#include <private/include/pch.h>
#include <private/include/CVulkanBufferObject.h>
#include <private/include/CVulkanThreadContext.h>

#include "private/include/CVulkanApplication.h"

namespace graphics_backend
{
	void* CVulkanBufferObject::GetMappedPointer() const
	{
		return m_BufferAllocationInfo.pMappedData;
	}
	//void CVulkanBufferObject::Release()
	//{
	//	if(m_Buffer != vk::Buffer(nullptr) && m_BufferAllocation != nullptr)
	//	{
	//		CVulkanMemoryManager& memoryManager = GetVulkanApplication().GetMemoryManager();
	//		memoryManager.ReleaseBuffer(*this);
	//	}
	//	m_Buffer = nullptr;
	//	m_BufferAllocation = nullptr;
	//	m_BufferAllocationInfo = {};
	//	m_OwningThreadContextId = std::numeric_limits<uint32_t>::max();
	//	m_OwningFrameBoundPoolId = INVALID_INDEX;
	//}
}
