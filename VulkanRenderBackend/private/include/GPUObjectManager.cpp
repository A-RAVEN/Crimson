#include "private/include/pch.h"
#include "GPUObjectManager.h"

namespace graphics_backend
{
	GPUObjectManager::GPUObjectManager(CVulkanApplication& application)
		: BaseApplicationSubobject(application)
		, m_ShaderModuleCache(application)
		, m_RenderPassCache(application)
		, m_FramebufferObjectCache(application)
		, m_PipelineObjectCache(application)
		, m_ShaderDescriptorPoolCache(application)
	{
	}
}

