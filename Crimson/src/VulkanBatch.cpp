#include <headers/VulkanBatch.h>
#include <headers/VulkanDebugLog.h>
#include <algorithm>

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
		CHECK_VKRESULT(vkCreateFence(p_OwningDevice->m_LogicalDevice, &fence_create_info, VULKAN_ALLOCATOR_POINTER, &m_Fence), "Vulkan Batch Create Fence Issue!");

		VkSemaphoreCreateInfo semaphore_create_info{};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphore_create_info.flags = 0;
		semaphore_create_info.pNext = nullptr;
		CHECK_VKRESULT(vkCreateSemaphore(p_OwningDevice->m_LogicalDevice, &semaphore_create_info, VULKAN_ALLOCATOR_POINTER, &m_Semaphore), "Vulkan Batch Create Semaphore Issue!");

		vkGetDeviceQueue(p_OwningDevice->m_LogicalDevice, m_QueueFamily, m_QueuePriority, &m_Queue);
	}
	void VulkanBatch::SubmitCommands()
	{
		//TODO Find a better way for submission like double buffer
		CHECK_VKRESULT(vkWaitForFences(p_OwningDevice->m_LogicalDevice, 1, &m_Fence, VK_TRUE, UINT_MAX), "Vulkan Batch Wait For Fence Issue Before Submit!");
		CHECK_VKRESULT(vkResetFences(p_OwningDevice->m_LogicalDevice, 1, &m_Fence), "Vulkan Batch Reset Fence Issue Before Submit!");
		std::vector<VkSemaphore> waiting_semaphores;
		std::vector<VkPipelineStageFlags> waiting_stages;
		std::vector<VkCommandBuffer> cmd_buffers = p_OwningDevice->CollectBatchCommandBuffers(m_BatchID, waiting_semaphores, waiting_stages);
		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = cmd_buffers.size();
		submit_info.pCommandBuffers = cmd_buffers.data();
		//TODO: Further extensions
		submit_info.pWaitDstStageMask = waiting_stages.size() > 0 ? waiting_stages.data() : nullptr;
		submit_info.pWaitSemaphores = waiting_semaphores.size() > 0 ? waiting_semaphores.data() : nullptr;
		submit_info.waitSemaphoreCount = waiting_semaphores.size();
		submit_info.signalSemaphoreCount = 0;
		submit_info.pSignalSemaphores = nullptr;
		submit_info.pNext = nullptr;

		CHECK_VKRESULT(vkQueueSubmit(m_Queue, 1, &submit_info, m_Fence), "Vulkan Queue Submit Batch Issue!");
	}
	void VulkanBatch::SubmitCommands(VkQueue queue)
	{
		//TODO Find a better way for submission like double buffer
		CHECK_VKRESULT(vkWaitForFences(p_OwningDevice->m_LogicalDevice, 1, &m_Fence, VK_TRUE, UINT_MAX), "Vulkan Batch Wait For Fence Issue Before Submit!");
		CHECK_VKRESULT(vkResetFences(p_OwningDevice->m_LogicalDevice, 1, &m_Fence), "Vulkan Batch Reset Fence Issue Before Submit!");
		std::vector<VkSemaphore> waiting_semaphores;
		std::vector<VkPipelineStageFlags> waiting_stages;
		std::vector<VkCommandBuffer> cmd_buffers = p_OwningDevice->CollectBatchCommandBuffers(m_BatchID, waiting_semaphores, waiting_stages);
		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = cmd_buffers.size();
		submit_info.pCommandBuffers = cmd_buffers.data();
		//TODO: Further extensions
		submit_info.pWaitDstStageMask = waiting_stages.size() > 0 ? waiting_stages.data() : nullptr;
		submit_info.pWaitSemaphores = waiting_semaphores.size() > 0 ? waiting_semaphores.data() : nullptr;
		submit_info.waitSemaphoreCount = waiting_semaphores.size();
		submit_info.signalSemaphoreCount = 0;
		submit_info.pSignalSemaphores = nullptr;
		submit_info.pNext = nullptr;

		CHECK_VKRESULT(vkQueueSubmit(queue, 1, &submit_info, m_Fence), "Vulkan Queue Submit Batch Issue!");
	}
	void VulkanBatch::DestroyBatch()
	{
		CHECK_VKRESULT(vkWaitForFences(p_OwningDevice->m_LogicalDevice, 1, &m_Fence, VK_TRUE, UINT_MAX), "Vulkan Batch Wait For Fence Issue On Destroy!");
		vkDestroyFence(p_OwningDevice->m_LogicalDevice, m_Fence, VULKAN_ALLOCATOR_POINTER);
		vkDestroySemaphore(p_OwningDevice->m_LogicalDevice, m_Semaphore, VULKAN_ALLOCATOR_POINTER);
	}
	void VulkanBatch::Wait()
	{
		CHECK_VKRESULT(vkWaitForFences(p_OwningDevice->m_LogicalDevice, 1, &m_Fence, VK_TRUE, UINT_MAX), "Vulkan Batch Wait For Fence Issue!");
	}
}