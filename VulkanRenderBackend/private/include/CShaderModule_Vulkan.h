#pragma once
#include <RenderInterface/header/CShaderModule.h>
#include "VulkanApplicationSubobjectBase.h"

namespace graphics_backend
{
	class CShaderModule_Vulkan : public CShaderModule, public BaseApplicationSubobject
	{
	public:
		CShaderModule_Vulkan(CVulkanApplication& owner) : BaseApplicationSubobject(owner) {}
		vk::ShaderModule GetVulkanModule() const { return m_ThisShaderModule; }
		// 通过 CShaderModule 继承
		virtual void SetShaderProvider(std::shared_ptr<const ShaderProvider> provider) override;
		virtual void Submit() override;
		virtual bool Done() override;
		virtual void Release() override;
	private:
		std::shared_ptr<const ShaderProvider> m_OwningProvider = nullptr;
		vk::ShaderModule m_ThisShaderModule = nullptr;
	};
}