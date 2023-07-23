#pragma once
#include "VulkanApplicationSubobjectBase.h"
#include "RenderInterface/header/CShaderModule.h"

namespace graphics_backend
{
	class CShaderModuleObject : public BaseApplicationSubobject
	{
	public:
		CShaderModuleObject(std::shared_ptr<const ShaderProvider> provider, CVulkanApplication& application);
		void Create();
	private:
		std::shared_ptr<const ShaderProvider> m_Provider;
		vk::ShaderModule m_ShaderModule = nullptr;
	};
}
