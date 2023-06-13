#include <private/include/pch.h>
#include <header/MRenderBackend.h>
#include <private/include/CRenderBackend_Vulkan.h>
namespace graphics_backend
{
	CRenderBackend* NewBackend()
	{
		return new CRenderBackend_Vulkan();
	}
	void DeleteBackend(CRenderBackend* removingBackend)
	{
		delete static_cast<CRenderBackend_Vulkan*>(removingBackend);
	}
}