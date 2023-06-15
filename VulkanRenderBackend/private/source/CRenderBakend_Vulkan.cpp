#include <private/include/pch.h>
#include <private/include/CRenderBackend_Vulkan.h>

namespace graphics_backend
{
	void CRenderBackend_Vulkan::Initialize(std::string const& appName, std::string const& engineName)
	{
		CRenderGraph graph;
		int tmp;
		graph.AddPass(CRenderGraphPassConfig(), [tmp](CInlineCommandList& commandList)
			{

			});

		m_Application.InitApp(appName, engineName);
	}

	void CRenderBackend_Vulkan::Release()
	{
		m_Application.ReleaseApp();
	}
}