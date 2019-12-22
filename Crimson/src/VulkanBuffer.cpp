#include <headers/VulkanBuffer.h>

namespace Crimson
{
	VulkanBufferObject::VulkanBufferObject() :
		p_OwningDevice(nullptr),
		m_Buffer(VK_NULL_HANDLE),
		m_Allocation(VK_NULL_HANDLE),
		m_CurrentQueueFamily(~0u)
	{}
	void VulkanBufferObject::SetVulkanBuffer(VulkanGPUDevice* p_device, VkBuffer buffer, VmaAllocation allocation_info, uint64_t size, std::vector<EBufferUsage> const& usages, EMemoryType memory_type)
	{
		p_OwningDevice = p_device;
		m_Buffer = buffer;
		m_Allocation = allocation_info;
		m_BufferSize = size;
		m_BufferUsages = usages;
		m_MemoryType = memory_type;
	}
	void VulkanBufferObject::Dispose()
	{
		p_OwningDevice->HandleDisposedBuffer(this);
	}
}