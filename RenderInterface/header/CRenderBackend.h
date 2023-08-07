#pragma once

///define DEFINE_RENDERBACKEND_LOAD_FUNCTIONS to define library loading functions


#include <string>
#include <memory>
#include "Common.h"
#include "CGPUPrimitiveResource.h"
#include "GPUBuffer.h"
#include "CNativeRenderPassInfo.h"

namespace thread_management
{
	class CThreadManager;
}

namespace graphics_backend
{
	class CGPUTextureResource
	{
	public:
		virtual CTextureInfo GetTextureInfo() const = 0;
	};

	class CRenderBackend
	{
	public:
		virtual void Initialize(std::string const& appName, std::string const& engineName) = 0;
		virtual void InitializeThreadContextCount(thread_management::CThreadManager* threadManager, uint32_t threadContextCount) = 0;
		virtual void Release() = 0;
		virtual void NewWindow(uint32_t width, uint32_t height, std::string const& windowName) = 0;
		virtual bool AnyWindowRunning() = 0;
		virtual void TickWindows() = 0;
		virtual void StartCurrentFrame() = 0;
		virtual void EndCurrentFrame() = 0;
		virtual void ExecuteRenderPass(CRenderpassBuilder const& inRenderPass) = 0;
		virtual void RunGraphWithPresentTarget(std::string const& windowName) = 0;
		virtual std::shared_ptr<GPUBuffer> CreateGPUBuffer(EBufferUsageFlags usageFlags
			, uint64_t count
			, uint64_t stride) = 0;
	};
}



