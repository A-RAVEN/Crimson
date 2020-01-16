#include <headers/VulkanBatch.h>

namespace Crimson
{
	void VulkanBatch::SetBatch(VulkanGPUDevice* p_owning_device, uint32_t batch_id, uint32_t queue_family_id, uint32_t priority)
	{
		p_OwningDevice = p_owning_device;
		m_BatchID = batch_id;
		m_QueueFamily = queue_family_id;
		m_QueuePriority = (std::min)(p_OwningDevice->m_QueueNumbers[queue_family_id] - 1, priority);

		VkFenceCreateInfo fence_create_info{};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkCreateFence(p_OwningDevice->m_LogicalDevice, &fence_create_info, VULKAN_ALLOCATOR_POINTER, &m_Fence);

		VkSemaphoreCreateInfo semaphore_create_info{};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphore_create_info.flags = 0;
		semaphore_create_info.pNext = nullptr;
		vkCreateSemaphore(p_OwningDevice->m_LogicalDevice, &semaphore_create_info, VULKAN_ALLOCATOR_POINTER, &m_Semaphore);

		vkGetDeviceQueue(p_OwningDevice->m_LogicalDevice, m_QueueFamily, m_QueuePriority, &m_Queue);
	}
	void VulkanBatch::SubmitCommands()
	{
		//p_OwningDevice->m_QueueFamilies
		VkSubmitInfo submit_info{};

		vkQueueSubmit(m_Queue, 1, &submit_info, m_Fence);
	}
}