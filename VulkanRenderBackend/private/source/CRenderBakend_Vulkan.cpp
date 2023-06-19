#include <private/include/pch.h>
#include <private/include/CRenderBackend_Vulkan.h>


namespace graphics_backend
{
	void CRenderBackend_Vulkan::Initialize(std::string const& appName, std::string const& engineName)
	{
		m_Application.InitApp(appName, engineName);
	}

	void CRenderBackend_Vulkan::Release()
	{
		m_Application.ReleaseApp();
	}

	void CRenderBackend_Vulkan::NewWindow(uint32_t width, uint32_t height, std::string const& windowName)
	{
		m_Application.CreateWindowContext(windowName, width, height);
	}

	bool CRenderBackend_Vulkan::AnyWindowRunning()
	{
		return m_Application.AnyWindowRunning();
	}

	void CRenderBackend_Vulkan::TickWindows()
	{
		m_Application.TickWindowContexts();
	}
}
