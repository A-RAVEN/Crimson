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
		uint32_t AquireNextImage();
		VkImageSubresourceLayers GetSwapchainImageSubresourceLayers() const;
		VkImageSubresourceRange GetSwapchainImageSubresourceRange() const;
		void InitSurfaceContext(VulkanGPUDevice* p_device, VkSurfaceKHR surface);
		void RevalidateSurfaceContext();
		void Dispose();
		VulkanGPUDevice*		p_OwningDevice;
		VkSurfaceKHR			m_OwningSurface;
		std::vector<VkImage>	m_SwapchainImages;
		std::vector<bool>		m_SwapchainImageInitialized;
		VkSwapchainKHR			m_Swapchain;
		VkSurfaceFormatKHR		m_SurfaceFormat;
		VkPresentModeKHR		m_PresentMode;
		uint32_t				m_PresentFamily;
		VkQueue					m_PresentQueue;

		VkSemaphore				m_AquireFinishSemaphore;
		VkFence					m_AquireFinishFence;

		VkExtent2D				m_Extent;

		uint32_t				m_PresentImageId;
	};
}