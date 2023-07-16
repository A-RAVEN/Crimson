#pragma once
#include <RenderInterface/header/CShaderModule.h>
#include "VulkanApplicationSubobjectBase.h"

namespace graphics_backend
{
	class CShaderModule_Vulkan : public CShaderModule, public BaseApplicationSubobject
	{
	public:
		CShaderModule_Vulkan(CVulkanApplication& owner) : BaseApplicationSubobject(owner) {}
		//CShaderModule_Vulkan(CShaderModule_Vulkan const& other) = delete;
		//CShaderModule_Vulkan& operator=(CShaderModule_Vulkan const&) = delete;
		//CShaderModule_Vulkan(CShaderModule_Vulkan&& other) = default;
		//CShaderModule_Vulkan& operator=(CShaderModule_Vulkan&&) = default;
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