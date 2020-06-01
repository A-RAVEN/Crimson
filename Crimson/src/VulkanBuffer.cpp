#include <headers/VulkanBuffer.h>
#include <headers/VulkanDebugLog.h>
#include <headers/GeneralDebug.h>
#include <headers/vk_mem_alloc.h>
#include <headers/VulkanTranslator.h>

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
	void VulkanBufferObject::InitTexelBufferView(std::string const& name, EFormat format, uint64_t offset, uint64_t range)
	{
		auto find = m_BufferViews.find(name);
		CRIM_ASSERT_AND_RETURN_VOID(find == m_BufferViews.end(), "Buffer View With Name " + name + " Already Created!");
		VkBufferViewCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
		create_info.buffer = m_Buffer;
		create_info.flags = VK_NULL_HANDLE;
		create_info.format = TranslateImageFormatToVulkan(format);
		create_info.offset = static_cast<VkDeviceSize>(offset);
		create_info.range = range > 0 ? static_cast<VkDeviceSize>(range) : VK_WHOLE_SIZE;
		create_info.pNext = nullptr;
		VkBufferView new_buffer_view = VK_NULL_HANDLE;
		CHECK_VKRESULT(vkCreateBufferView(p_OwningDevice->m_LogicalDevice, &create_info, VULKAN_ALLOCATOR_POINTER, &new_buffer_view), "Vulkan Create Buffer View Issue!");
		m_BufferViews.insert(std::make_pair(name, new_buffer_view));
	}
	VkBufferView VulkanBufferObject::GetVulkanBufferView(std::string const& name)
	{
		auto find = m_BufferViews.find(name);
		if (find != m_BufferViews.end())
		{
			return find->second;
		}
		return VK_NULL_HANDLE;
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
			CHECK_VKRESULT(vmaMapMemory(p_OwningDevice->m_MemoryAllocator, m_Allocation, &p_data), "Vulkan Map Memory Issue!");
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