#include <private/include/pch.h>
#include <private/include/CVulkanBufferObject.h>
#include <private/include/CVulkanThreadContext.h>

#include "private/include/CVulkanApplication.h"

namespace graphics_backend
{
	CVulkanBufferObject::CVulkanBufferObject(CVulkanApplication& owner) : 
		BaseApplicationSubobject(owner)
	{
	}
	void CVulkanBufferObject::Initialize(vk::Buffer const& buffer, VmaAllocation const& allocation, VmaAllocationInfo const& allocationInfo)
	{
		m_Buffer = buffer;
		m_BufferAllocation = allocation;
		m_BufferAllocationInfo = allocationInfo;
	}
	void* CVulkanBufferObject::GetMappedPointer() const
	{
		return m_BufferAllocationInfo.pMappedData;
	}
	void CVulkanBufferObject::Release()
	{
		m_Buffer = nullptr;
		m_BufferAllocation = nullptr;
		m_BufferAllocationInfo = {};
		m_OwningFrameBoundPoolId = INVALID_INDEX;
	}
}
