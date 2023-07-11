#include <private/include/pch.h>
#include <private/include/WindowContext.h>
#include <math.h>
#include "private/include/CVulkanApplication.h"

namespace graphics_backend
{
	class glfwContext
	{
	public:
		glfwContext()
		{
			glfwInit();
			glfwSetErrorCallback([](int error, const char* msg)
				{
					std::cerr << "glfw Error: " << "(" << error << ")" << msg << std::endl;
				});
		}

		~glfwContext()
		{
			glfwTerminate();
		}
	};

	static glfwContext s_GLFWContext = glfwContext();

	CWindowContext::CWindowContext(std::string const& windowName, uint32_t initialWidth, uint32_t initialHeight) :
		m_WindowName(windowName), m_Width(initialWidth), m_Height(initialHeight)
	{
	}

	bool CWindowContext::NeedClose() const
	{
		assert(ValidContext());
		if(m_Window != nullptr)
		{
			return glfwWindowShouldClose(m_Window);
		}
		return false;
	}

	void CWindowContext::Initialize_Internal(CVulkanApplication const* owningApplication)
	{
		assert(ValidContext());
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_Window = glfwCreateWindow(m_Width, m_Height, m_WindowName.c_str(), nullptr, nullptr);
		VkSurfaceKHR surface;
		glfwCreateWindowSurface(static_cast<VkInstance>(m_OwningApplication->GetInstance()), m_Window, nullptr, &surface);
		m_Surface = vk::SurfaceKHR(surface);

		m_PresentQueue = GetVulkanApplication()->GetSubmitCounterContext().FindPresentQueue(m_Surface);

		std::vector<vk::SurfaceFormatKHR> formats = GetPhysicalDevice().getSurfaceFormatsKHR(surface);
		assert(!formats.empty());
		vk::Format format = (formats[0].format == vk::Format::eUndefined) ? vk::Format::eB8G8R8A8Unorm : formats[0].format;
		vk::SurfaceCapabilitiesKHR surfaceCapabilities = GetPhysicalDevice().getSurfaceCapabilitiesKHR(surface);
		vk::Extent2D               swapchainExtent;
		if (surfaceCapabilities.currentExtent.width == std::numeric_limits<uint32_t>::max())
		{
			// If the surface size is undefined, the size is set to the size of the images requested.
			swapchainExtent.width =  std::min(std::max(m_Width, surfaceCapabilities.minImageExtent.width), surfaceCapabilities.maxImageExtent.width);
			swapchainExtent.height = std::min(std::max(m_Height, surfaceCapabilities.minImageExtent.height), surfaceCapabilities.maxImageExtent.height);
		}
		else
		{
			// If the surface size is defined, the swap chain size must match
			swapchainExtent = surfaceCapabilities.currentExtent;
		}

		// The FIFO present mode is guaranteed by the spec to be supported
		vk::PresentModeKHR swapchainPresentMode = vk::PresentModeKHR::eFifo;

		vk::SurfaceTransformFlagBitsKHR preTransform = (surfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
			? vk::SurfaceTransformFlagBitsKHR::eIdentity
			: surfaceCapabilities.currentTransform;

		vk::CompositeAlphaFlagBitsKHR compositeAlpha =
			(surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied) ? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
			: (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied) ? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
			: (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit) ? vk::CompositeAlphaFlagBitsKHR::eInherit
			: vk::CompositeAlphaFlagBitsKHR::eOpaque;

		vk::SwapchainCreateInfoKHR swapChainCreateInfo(vk::SwapchainCreateFlagsKHR(),
			surface,
			surfaceCapabilities.minImageCount,
			format,
			vk::ColorSpaceKHR::eSrgbNonlinear,
			swapchainExtent,
			1,
			vk::ImageUsageFlagBits::eColorAttachment,
			vk::SharingMode::eExclusive,
			{},
			preTransform,
			compositeAlpha,
			swapchainPresentMode,
			true,
			nullptr);

		uint32_t graphicsFamily = GetVulkanApplication()->GetSubmitCounterContext().GetGraphicsQueueRef().first;

		uint32_t queueFamilyIndices[2] = { graphicsFamily, m_PresentQueue.first };
		if (graphicsFamily != m_PresentQueue.first)
		{
			// If the graphics and present queues are from different queue families, we either have to explicitly transfer
			// ownership of images between the queues, or we have to create the swapchain with imageSharingMode as
			// VK_SHARING_MODE_CONCURRENT
			swapChainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			swapChainCreateInfo.queueFamilyIndexCount = 2;
			swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
		}

		m_Swapchain = GetDevice().createSwapchainKHR(swapChainCreateInfo);
		m_SwapchainImages = GetDevice().getSwapchainImagesKHR(m_Swapchain);
	}

	void CWindowContext::Release_Internal()
	{
		if(m_Surface != vk::SurfaceKHR(nullptr))
		{
			m_OwningApplication->GetInstance().destroySurfaceKHR(m_Surface);
			m_Surface = nullptr;
		}
		if(m_Window != nullptr)
		{
			glfwDestroyWindow(m_Window);
			m_Window = nullptr;
		}
		m_PresentQueue = std::pair<uint32_t, vk::Queue>(INVALID_INDEX, nullptr);

		m_Swapchain = nullptr;
		m_SwapchainImages.clear();
	}
}
