#include "private/include/pch.h"
#include "VulkanImageObject.h"

namespace graphics_backend
{
	VulkanImageObject_Internal::VulkanImageObject_Internal(
		GPUTextureDescriptor const& descriptor
		, vk::Image const& image
		, VmaAllocation const& allocation
		, VmaAllocationInfo const& allocationInfo
	): 
		m_Image(image)
		, m_ImageAllocation(allocation)
		, m_ImageAllocationInfo(allocationInfo)
		, m_Descriptor(descriptor)
	{
	}
	void* VulkanImageObject_Internal::GetMappedPointer() const
	{
		return m_ImageAllocationInfo.pMappedData;
	}
}