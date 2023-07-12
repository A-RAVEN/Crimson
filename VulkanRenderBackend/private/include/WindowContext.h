#pragma once
#include <private/include/VulkanApplicationSubobjectBase.h>
#include "RenderInterface/header/Common.h"

namespace graphics_backend
{
	class CWindowContext : public ApplicationSubobjectBase
	{
	public:
		inline bool ValidContext() const { return m_Width > 0 && m_Height > 0; }
		CWindowContext(std::string const& windowName, uint32_t initialWidth, uint32_t initialHeight);
		bool NeedClose() const;
		std::string GetName() const { return m_WindowName; }
	protected:
		void Initialize_Internal(CVulkanApplication const* owningApplication) override;
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
		vk::Semaphore m_WaitNextFrameSemaphore = nullptr;

		friend class CVulkanApplication;
	};
}
