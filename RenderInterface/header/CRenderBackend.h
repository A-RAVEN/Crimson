#pragma once

///define DEFINE_RENDERBACKEND_LOAD_FUNCTIONS to define library loading functions


#include <string>
#include "Common.h"
#include "CGPUPrimitiveResource.h"

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
		virtual CGPUPrimitiveResource* NewGPUPrimitiveResource() = 0;
		virtual void ReleaseGPUPrimitiveResource(CGPUPrimitiveResource* resource) = 0;
	};

	extern "C"
	{
		RENDERBACKEND_API CRenderBackend* NewRenderBackend();
		RENDERBACKEND_API void DeleteRenderBackend(CRenderBackend* removingBackend);
	}

	typedef CRenderBackend* (*FPT_NewBackend)();
	typedef void(*FPT_DeleteBackend)(CRenderBackend*);

#ifndef ANY_RENDER_BACKEND_EXPORTS
	void LoadRenderBackend();
	void UnloadRenderBackend();
#endif

#if defined(DEFINE_RENDERBACKEND_LOAD_FUNCTIONS) && !defined(ANY_RENDER_BACKEND_EXPORTS)

	HINSTANCE hRenderBackendLib = nullptr;
	FPT_NewBackend pNewBackendFunc = nullptr;
	FPT_DeleteBackend pDeleteBackendFunc = nullptr;

	void LoadRenderBackend()
	{
		hRenderBackendLib = LoadLibrary(L"VulkanRenderBackend");
		if (hRenderBackendLib != nullptr)
		{
			pNewBackendFunc = reinterpret_cast<FPT_NewBackend>(GetProcAddress(hRenderBackendLib, "NewRenderBackend"));
			pDeleteBackendFunc = reinterpret_cast<FPT_DeleteBackend>(GetProcAddress(hRenderBackendLib, "DeleteRenderBackend"));
		}
	}

	void UnloadRenderBackend()
	{
		pNewBackendFunc = nullptr;
		pDeleteBackendFunc = nullptr;
		if (hRenderBackendLib != nullptr)
		{
			FreeLibrary(hRenderBackendLib);
			hRenderBackendLib = nullptr;
		}
	}

	CRenderBackend* NewRenderBackend()
	{
		return pNewBackendFunc();
	}

	void DeleteRenderBackend(CRenderBackend* removingBackend)
	{
		pDeleteBackendFunc(removingBackend);
	}
#endif
}



