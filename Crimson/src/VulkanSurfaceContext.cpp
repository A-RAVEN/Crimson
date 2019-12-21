#include <headers/VulkanSurfaceContext.h>
#include <headers/VulkanGPUDevice.h>
#include <headers/VulkanDebugLog.h>
#include <headers/VulkanHeader.h>
#include <algorithm>

namespace Crimson
{
	static VkSurfaceFormatKHR SelectDesiredFormat(std::vector<VkSurfaceFormatKHR> const& formats)
	{
		if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
		{
			return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
		}
		for (const auto& itr_format : formats) {
			if (itr_format.format == VK_FORMAT_R8G8B8A8_UNORM && itr_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return itr_format;
			}
		}
		return formats[0];
	}

	static VkPresentModeKHR SelectPresentMode(std::vector<VkPresentModeKHR> const& present_modes)
	{
		for (auto present_mode : present_modes)
		{
			if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return present_mode;
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VulkanSurfaceContext::VulkanSurfaceContext() : 
		p_OwningDevice(nullptr),
		m_OwningSurface(VK_NULL_HANDLE),
		m_Swapchain(VK_NULL_HANDLE),
		m_SurfaceFormat{ VK_FORMAT_UNDEFINED , VK_COLOR_SPACE_MAX_ENUM_KHR },
		m_PresentMode(VK_PRESENT_MODE_FIFO_KHR),
		m_PresentFamily(std::numeric_limits<uint32_t>::max())
	{}
	VulkanSurfaceContext::~VulkanSurfaceContext(){}
	void VulkanSurfaceContext::InitSurfaceContext(VulkanGPUDevice* p_device, VkSurfaceKHR surface)
	{
		p_OwningDevice = p_device;
		m_OwningSurface = surface;
		VkSurfaceCapabilitiesKHR capabilities{};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(p_device->m_PhysicalDevice, surface, &capabilities);

		{
			//check for format count and present mode count
			uint32_t count;
			vkGetPhysicalDeviceSurfaceFormatsKHR(p_device->m_PhysicalDevice, surface, &count, nullptr);
			std::vector<VkSurfaceFormatKHR> formats(count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(p_device->m_PhysicalDevice, surface, &count, formats.data());

			vkGetPhysicalDeviceSurfacePresentModesKHR(p_device->m_PhysicalDevice, surface, &count, nullptr);
			std::vector<VkPresentModeKHR> presenmt_mods(count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(p_device->m_PhysicalDevice, surface, &count, presenmt_mods.data());

			m_SurfaceFormat = SelectDesiredFormat(formats);
			m_PresentMode = SelectPresentMode(presenmt_mods);
		}

		for (uint32_t family_id = 0; family_id < p_device->m_QueueFamilies.size(); ++family_id)
		{
			if (p_device->m_QueueNumbers[family_id] > 0)
			{
				VkBool32 b_support_surface = VK_FALSE;
				VulkanDebug::CheckVKResult(vkGetPhysicalDeviceSurfaceSupportKHR(p_device->m_PhysicalDevice, family_id, surface, &b_support_surface), "Vulkan Check Surface Support Issue!");
				if (b_support_surface == VK_TRUE)
				{
					m_PresentFamily = family_id;
					VkSwapchainCreateInfoKHR swapchain_create_info{};
					swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
					swapchain_create_info.pNext = nullptr;
					swapchain_create_info.surface = m_OwningSurface;
					//swapchain image
					swapchain_create_info.imageFormat = m_SurfaceFormat.format;
					swapchain_create_info.imageColorSpace = m_SurfaceFormat.colorSpace;
					swapchain_create_info.imageArrayLayers = 1;
					swapchain_create_info.minImageCount = (std::min)({ capabilities.minImageCount + 1, capabilities.maxImageCount });
					swapchain_create_info.imageExtent.width = capabilities.currentExtent.width;
					swapchain_create_info.imageExtent.height = capabilities.currentExtent.height;
					swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
					swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
					swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

					swapchain_create_info.preTransform = capabilities.currentTransform;
					swapchain_create_info.presentMode = m_PresentMode;
					swapchain_create_info.queueFamilyIndexCount = 0;
					swapchain_create_info.pQueueFamilyIndices = nullptr;
					swapchain_create_info.clipped = VK_TRUE;

					VulkanDebug::CheckVKResult(vkCreateSwapchainKHR(p_device->m_LogicalDevice, &swapchain_create_info, VULKAN_ALLOCATOR_POINTER, &m_Swapchain), "Vulkan Swapchain Creation Issue!");
					return;
				}
			}
		}
	}
	void VulkanSurfaceContext::RevalidateSurfaceContext()
	{
	}
	void VulkanSurfaceContext::Dispose()
	{
	}
}