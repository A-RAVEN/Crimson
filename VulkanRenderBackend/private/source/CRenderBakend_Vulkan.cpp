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

	std::shared_ptr<WindowHandle> CRenderBackend_Vulkan::NewWindow(uint32_t width, uint32_t height, std::string const& windowName)
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

	void CRenderBackend_Vulkan::ExecuteRenderPass(CRenderpassBuilder const& inRenderPass)
	{
		m_Application.ExecuteRenderPass(inRenderPass);
	}

	void CRenderBackend_Vulkan::RunGraphWithPresentTarget(std::string const& windowName)
	{
		m_Application.RunGraphWithPresentTarget(windowName);
	}

	std::shared_ptr<GPUBuffer> CRenderBackend_Vulkan::CreateGPUBuffer(EBufferUsageFlags usageFlags, uint64_t count, uint64_t stride)
	{
		return std::shared_ptr<GPUBuffer>(m_Application.NewGPUBuffer(usageFlags
			, count
			, stride), [this](GPUBuffer* releaseBuffer)
			{
				m_Application.ReleaseGPUBuffer(releaseBuffer);
			});
	}
}
