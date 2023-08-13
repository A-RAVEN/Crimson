#pragma once
#include <private/include/VulkanApplicationSubobjectBase.h>
#include "RenderInterface/header/Common.h"
#include <RenderInterface/header/WindowHandle.h>

namespace graphics_backend
{
	class CWindowContext : public BaseApplicationSubobject, public WindowHandle
	{
	public:
		inline bool ValidContext() const { return m_Width > 0 && m_Height > 0; }
		CWindowContext(CVulkanApplication& inOwner);
		bool NeedClose() const;
		std::string GetName() const { return m_WindowName; }
		void WaitCurrentFrameBufferIndex();
		TIndex GetCurrentFrameBufferIndex() const { return m_CurrentBufferIndex; }
		vk::Image GetCurrentFrameImage() const { return m_SwapchainImages[m_CurrentBufferIndex]; }
		vk::ImageView GetCurrentFrameImageView() const { return m_SwapchainImageViews[m_CurrentBufferIndex]; }
		void Initialize(std::string const& windowName, uint32_t initialWidth, uint32_t initialHeight);
		void Release_Internal() override;
	private:
		std::string m_WindowName;
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;

		GLFWwindow* m_Window = nullptr;
		vk::SurfaceKHR m_Surface = nullptr;
		std::pair<uint32_t, vk::Queue> m_PresentQueue = std::pair<uint32_t, vk::Queue>(INVALID_INDEX, nullptr);
		vk::SwapchainKHR m_Swapchain = nullptr;
		std::vector<vk::Image> m_SwapchainImages;
		std::vector<vk::ImageView> m_SwapchainImageViews;
		vk::Semaphore m_WaitNextFrameSemaphore = nullptr;
		vk::Semaphore m_CanPresentSemaphore = nullptr;

		TIndex m_CurrentBufferIndex = INVALID_INDEX;

		friend class CVulkanApplication;
	};
}
