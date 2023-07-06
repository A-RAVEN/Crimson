#include <private/include/pch.h>
#include <private/include/WindowContext.h>

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
		m_PresentQueue = nullptr;
	}
}
