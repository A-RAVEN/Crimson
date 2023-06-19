#pragma once
#include <private/include/VulkanApplicationSubobjectBase.h>

namespace graphics_backend
{
	class CWindowContext : ApplicationSubobjectBase
	{
	public:
		inline bool ValidContext() const { return m_Width > 0 && m_Height > 0; }
	protected:
		void Initialize_Internal(CVulkanApplication const* owningApplication) override;
		void Release_Internal() override;
	private:
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
		std::string m_WindowName;

		GLFWwindow* m_Window = nullptr;
		vk::SurfaceKHR m_Surface = nullptr;
	};
}