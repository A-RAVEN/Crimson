#pragma once
#include <headers/VulkanHeader.h>

namespace Crimson
{
	class VulkanGPUDevice;
	class VulkanSurfaceContext
	{
	public:
		friend class VulkanGPUDevice;
		VulkanSurfaceContext();
		~VulkanSurfaceContext();
	private:
		void InitSurfaceContext(VulkanGPUDevice* p_device, VkSurfaceKHR surface);
		void RevalidateSurfaceContext();
		void Dispose();
		VulkanGPUDevice*	p_OwningDevice;
		VkSurfaceKHR		m_OwningSurface;
		VkSwapchainKHR		m_Swapchain;
		VkSurfaceFormatKHR	m_SurfaceFormat;
		VkPresentModeKHR	m_PresentMode;
		uint32_t			m_PresentFamily;
	};
}