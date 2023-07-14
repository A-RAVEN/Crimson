#include <private/include/pch.h>
#include <private/include/CRenderBackend_Vulkan.h>
#include <RenderInterface/header/CGPUPrimitiveResource.h>


namespace graphics_backend
{
	void CRenderBackend_Vulkan::Initialize(std::string const& appName, std::string const& engineName)
	{
		m_Application.InitApp(appName, engineName);
	}

	void CRenderBackend_Vulkan::InitializeThreadContextCount(CThreadManager* threadManager, uint32_t threadCount)
	{
		m_Application.InitializeThreadContext(threadManager, threadCount);
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
	void CRenderBackend_Vulkan::StartCurrentFrame()
	{
		m_Application.PrepareBeforeTick();
	}
	void CRenderBackend_Vulkan::EndCurrentFrame()
	{
		m_Application.EndThisFrame();
	}

	void CRenderBackend_Vulkan::RunGraphWithPresentTarget(std::string const& windowName)
	{
		m_Application.RunGraphWithPresentTarget(windowName);
	}

	CGPUPrimitiveResource* CRenderBackend_Vulkan::NewGPUPrimitiveResource()
	{
		return m_Application.NewPrimitiveResource();
	}

	void CRenderBackend_Vulkan::ReleaseGPUPrimitiveResource(CGPUPrimitiveResource* resource)
	{
		m_Application.DestroyPrimitiveResource(static_cast<CGPUPrimitiveResource_Vulkan*>(resource));
	}
}