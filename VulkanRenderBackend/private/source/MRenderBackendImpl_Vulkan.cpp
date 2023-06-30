#include <private/include/pch.h>
#include <RenderInterface/header/CRenderBackend.h>
#include <private/include/CRenderBackend_Vulkan.h>

namespace graphics_backend
{
	CRenderBackend* NewRenderBackend()
	{
		return new graphics_backend::CRenderBackend_Vulkan();
	}
	void DeleteRenderBackend(graphics_backend::CRenderBackend* removingBackend)
	{
		delete static_cast<graphics_backend::CRenderBackend_Vulkan*>(removingBackend);
	}
}
