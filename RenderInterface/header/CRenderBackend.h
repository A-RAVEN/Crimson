#pragma once

///define DEFINE_RENDERBACKEND_LOAD_FUNCTIONS to define library loading functions


#include <string>
#include "Common.h"
#include "CGPUPrimitiveResource.h"
#include "CShaderModule.h"

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
		virtual void RunGraphWithPresentTarget(std::string const& windowName) = 0;
		virtual CGPUPrimitiveResource* NewGPUPrimitiveResource() = 0;
		virtual void ReleaseGPUPrimitiveResource(CGPUPrimitiveResource* resource) = 0;
	};
}



