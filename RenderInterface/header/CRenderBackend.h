#pragma once

///define DEFINE_RENDERBACKEND_LOAD_FUNCTIONS to define library loading functions


#include <string>
#include <memory>
#include "Common.h"
#include "GPUBuffer.h"
#include "CNativeRenderPassInfo.h"
#include "WindowHandle.h"
#include "CRenderGraph.h"
#include "ShaderBindingSet.h"

namespace thread_management
{
	class CThreadManager;
}

namespace graphics_backend
{
	class CRenderBackend
	{
	public:
		virtual void Initialize(std::string const& appName, std::string const& engineName) = 0;
		virtual void InitializeThreadContextCount(thread_management::CThreadManager* threadManager, uint32_t threadContextCount) = 0;
		virtual void Release() = 0;
		virtual std::shared_ptr<WindowHandle> NewWindow(uint32_t width, uint32_t height, std::string const& windowName) = 0;
		virtual bool AnyWindowRunning() = 0;
		virtual void TickWindows() = 0;
		virtual void StartCurrentFrame() = 0;
		virtual void EndCurrentFrame() = 0;
		virtual void ExecuteRenderPass(CRenderpassBuilder const& inRenderPass) = 0;
		virtual void ExecuteRenderGraph(std::shared_ptr<CRenderGraph> inRenderGraph) = 0;
		virtual std::shared_ptr<GPUBuffer> CreateGPUBuffer(EBufferUsageFlags usageFlags
			, uint64_t count
			, uint64_t stride) = 0;
	};
}



