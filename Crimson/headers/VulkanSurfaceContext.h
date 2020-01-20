#pragma once
#include <headers/VulkanHeader.h>
#include <vector>

namespace Crimson
{
	class VulkanGPUDevice;
	class VulkanSurfaceContext
	{
	public:
		friend class VulkanGPUDevice;
		friend class VulkanExecutionCommandBuffer;
		VulkanSurfaceContext();
		~VulkanSurfaceContext();
	private:
		VkImage AquireNextImage();
		VkImageSubresourceLayers GetSwapchainImageSubresourceLayers();
		void InitSurfaceContext(VulkanGPUDevice* p_device, VkSurfaceKHR surface);
		void RevalidateSurfaceContext();
		void Dispose();
		VulkanGPUDevice*		p_OwningDevice;
		VkSurfaceKHR			m_OwningSurface;
		std::vector<VkImage>	m_SwapchainImages;
		VkSwapchainKHR			m_Swapchain;
		VkSurfaceFormatKHR		m_SurfaceFormat;
		VkPresentModeKHR		m_PresentMode;
		uint32_t				m_PresentFamily;

		VkSemaphore				m_AquireFinishSemaphore;
		VkFence					m_AquireFinishFence;
	};
}