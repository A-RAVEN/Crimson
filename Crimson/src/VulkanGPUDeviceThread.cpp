#include <headers/VulkanGPUDeviceThread.h>
#include <headers/VulkanGPUDevice.h>
#include <headers/VulkanDebugLog.h>

namespace Crimson
{
	PGraphicsCommandBuffer VulkanGPUDeviceThread::StartSubpassCommand(PRenderPassInstance renderpass_instance, uint32_t subpass_id)
	{

		return PGraphicsCommandBuffer();
	}
	void VulkanGPUDeviceThread::InitGPUDeviceThread(VulkanGPUDevice* device)
	{
		p_OwningDevice = device;
	}
	void VulkanGPUDeviceThread::InitGraphicsCommandPool()
	{
		VkCommandPoolCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		create_info.queueFamilyIndex = p_OwningDevice->m_GraphicsDedicateFamily;
		create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		create_info.pNext = nullptr;
		VulkanDebug::CheckVKResult(vkCreateCommandPool(p_OwningDevice->m_LogicalDevice, &create_info,
			VULKAN_ALLOCATOR_POINTER, &m_GraphicsCommandPool), "Vulkan Create Graphics Command Pool Issue!");
	}
	void VulkanGPUDeviceThread::InitComputeCommandPool()
	{
		VkCommandPoolCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		create_info.queueFamilyIndex = p_OwningDevice->m_ComputeDedicateFamily;
		create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		create_info.pNext = nullptr;
		VulkanDebug::CheckVKResult(vkCreateCommandPool(p_OwningDevice->m_LogicalDevice, &create_info,
			VULKAN_ALLOCATOR_POINTER, &m_ComputeCommandPool), "Vulkan Create Compute Command Pool Issue!");
	}
}