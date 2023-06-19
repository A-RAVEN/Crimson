#pragma once
#include <private/include/VulkanApplicationSubobjectBase.h>

namespace graphics_backend
{
	class CWindowContext : public ApplicationSubobjectBase
	{
	public:
		inline bool ValidContext() const { return m_Width > 0 && m_Height > 0; }
		CWindowContext(std::string const& windowName, uint32_t initialWidth, uint32_t initialHeight);
		bool NeedClose() const;
	protected:
		void Initialize_Internal(CVulkanApplication const* owningApplication) override;
		void Release_Internal() override;
	private:
		std::string m_WindowName;
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;

		GLFWwindow* m_Window = nullptr;
		vk::SurfaceKHR m_Surface = nullptr;
	};
}