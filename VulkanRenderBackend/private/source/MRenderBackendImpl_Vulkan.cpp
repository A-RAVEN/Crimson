#include <private/include/pch.h>
#include <RenderInterface/header/CRenderBackend.h>
#include <private/include/CRenderBackend_Vulkan.h>

#ifdef VULKANRENDERBACKEND_EXPORTS
#define RENDERINTERFACE_API __declspec(dllexport)
#else
#define RENDERINTERFACE_API
#endif


namespace graphics_backend
{
	extern "C"
	{
		RENDERINTERFACE_API CRenderBackend* NewModuleInstance()
		{
			return new graphics_backend::CRenderBackend_Vulkan();
		}
		RENDERINTERFACE_API void DeleteModuleInstance(graphics_backend::CRenderBackend* removingBackend)
		{
			delete static_cast<graphics_backend::CRenderBackend_Vulkan*>(removingBackend);
		}
	}
}
