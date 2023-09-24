#include <private/include/pch.h>
#include <private/include/CRenderBackend_Vulkan.h>


namespace graphics_backend
{
	void CRenderBackend_Vulkan::Initialize(std::string const& appName, std::string const& engineName)
	{
		m_Application.InitApp(appName, engineName);
	}

	void CRenderBackend_Vulkan::InitializeThreadContextCount(CThreadManager* threadManager, uint32_t threadCount)
	{
		m_Application.InitializeThreadContext(threadManager, threadCount);
		m_Application.PrepareBeforeTick();
	}

	void CRenderBackend_Vulkan::Release()
	{
		m_Application.ReleaseApp();
	}

	std::shared_ptr<WindowHandle> CRenderBackend_Vulkan::NewWindow(uint32_t width, uint32_t height, std::string const& windowName)
	{
		return m_Application.CreateWindowContext(windowName, width, height);
	}

	bool CRenderBackend_Vulkan::AnyWindowRunning()
	{
		return m_Application.AnyWindowRunning();
	}

	void CRenderBackend_Vulkan::TickWindows()
	{
		m_Application.TickWindowContexts();
	}
	void CRenderBackend_Vulkan::TickBackend()
	{
		m_Application.EndThisFrame();
		m_Application.PrepareBeforeTick();
	}

	void CRenderBackend_Vulkan::ExecuteRenderGraph(std::shared_ptr<CRenderGraph> inRenderGraph)
	{
		m_Application.ExecuteRenderGraph(inRenderGraph);
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
	std::shared_ptr<ShaderConstantSet> CRenderBackend_Vulkan::CreateShaderConstantSet(ShaderConstantsBuilder const& inBuilder)
	{
		return m_Application.NewShaderConstantSet(inBuilder);
	}
	std::shared_ptr<ShaderBindingSet> CRenderBackend_Vulkan::CreateShaderBindingSet(ShaderBindingBuilder const& inBuilder)
	{
		return m_Application.NewShaderBindingSet(inBuilder);
	}
}
