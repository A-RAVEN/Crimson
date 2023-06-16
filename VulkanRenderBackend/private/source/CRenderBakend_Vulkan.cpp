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
}