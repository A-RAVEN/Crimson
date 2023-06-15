#include <private/include/pch.h>
#include <header/MRenderBackend.h>
#include <private/include/CRenderBackend_Vulkan.h>


graphics_backend::CRenderBackend* NewBackend()
{
	return new graphics_backend::CRenderBackend_Vulkan();
}
void DeleteBackend(graphics_backend::CRenderBackend* removingBackend)
{
	delete static_cast<graphics_backend::CRenderBackend_Vulkan*>(removingBackend);
}