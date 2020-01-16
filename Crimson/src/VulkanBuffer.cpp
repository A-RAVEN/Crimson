#include <headers/VulkanBuffer.h>
#include <headers/VulkanDebugLog.h>
#include <headers/vk_mem_alloc.h>

namespace Crimson
{
	VulkanBufferObject::VulkanBufferObject() :
		p_OwningDevice(nullptr),
		m_Buffer(VK_NULL_HANDLE),
		m_Allocation(VK_NULL_HANDLE),
		m_CurrentQueueFamily(UNDEFINED_QUEUE_FAMILY_ID),
		p_Mapped(nullptr)
	{}
	void VulkanBufferObject::SetVulkanBuffer(VulkanGPUDevice* p_device, VkBuffer buffer, VmaAllocation allocation_info, uint64_t size, std::vector<EBufferUsage> const& usages, EMemoryType memory_type)
	{
		p_OwningDevice = p_device;
		m_Buffer = buffer;
		m_Allocation = allocation_info;
		m_BufferSize = size;
		m_BufferUsages = usages;
		m_MemoryType = memory_type;
		p_Mapped = nullptr;
	}
	void VulkanBufferObject::Dispose()
	{
		p_OwningDevice->HandleDisposedBuffer(this);
	}
	uint8_t* VulkanBufferObject::GetMappedPointer()
	{
		if ((p_Mapped == nullptr) && (m_MemoryType == EMemoryType::E_MEMORY_TYPE_HOST || m_MemoryType == EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE || m_MemoryType == EMemoryType::E_MEMORY_TYPE_DEVICE_TO_HOST))
		{
			void* p_data = nullptr;
			VulkanDebug::CheckVKResult(vmaMapMemory(p_OwningDevice->m_MemoryAllocator, m_Allocation, &p_data), "Vulkan Map Memory Issue!");
			p_Mapped = reinterpret_cast<uint8_t*>(p_data);
		}
		return p_Mapped;
	}
	void VulkanBufferObject::UnMapp()
	{
		if (p_Mapped != nullptr)
		{
			vmaUnmapMemory(p_OwningDevice->m_MemoryAllocator, m_Allocation);
			//vkUnmapMemory(p_OwningDevice->m_LogicalDevice, m_Allocation->GetMemory());
		}
		p_Mapped = nullptr;
	}
}