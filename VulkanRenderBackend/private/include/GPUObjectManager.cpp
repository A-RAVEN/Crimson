#include "private/include/pch.h"
#include "GPUObjectManager.h"

namespace graphics_backend
{
	GPUObjectManager::GPUObjectManager(CVulkanApplication& owner)
		: BaseApplicationSubobject(owner)
		, m_ShaderModuleCache(owner)
		, m_RenderPassCache(owner)
		, m_FramebufferObjectCache(owner)
		, m_PipelineObjectCache(owner)
	{
	}
}

