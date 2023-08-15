#include <private/include/pch.h>
#include <RenderInterface/header/CRenderBackend.h>
#include <private/include/CRenderBackend_Vulkan.h>

namespace graphics_backend
{
	CA_LIBRARY_INSTANCE_LOADING_FUNCTIONS(CRenderBackend, CRenderBackend_Vulkan)
}
