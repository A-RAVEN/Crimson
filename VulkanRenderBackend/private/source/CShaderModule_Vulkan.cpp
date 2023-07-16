#include "private/include/pch.h"
#include "private/include/CShaderModule_Vulkan.h"
#include <private/include/CVulkanApplication.h>

namespace graphics_backend
{
	void CShaderModule_Vulkan::SetShaderProvider(std::shared_ptr<const ShaderProvider> provider)
	{
		assert(provider != nullptr);
		m_OwningProvider = provider;
	}

	void CShaderModule_Vulkan::Submit()
	{
		assert(m_OwningProvider != nullptr);
		assert(m_ThisShaderModule == vk::ShaderModule(nullptr));
		GetVulkanApplication().NewTask()
			->Name("Create GPU ShaderModule")
			->Functor([this]()
				{
					const std::string codeType = "spirv";
					uint64_t length = m_OwningProvider->GetDataLength(codeType);
					uint32_t codeLength_integer = length / sizeof(uint32_t);
					std::vector<uint32_t> dataArray{codeLength_integer};
					memcpy(dataArray.data(), m_OwningProvider->GetDataPtr(codeType), length);
					vk::ShaderModuleCreateInfo shaderModuelCreateInfo(
						{}
						, dataArray
					);
					std::atomic_thread_fence(std::memory_order_release);
					m_ThisShaderModule = GetDevice().createShaderModule(shaderModuelCreateInfo);
				});
	}
	bool CShaderModule_Vulkan::Done()
	{
		return (m_ThisShaderModule != vk::ShaderModule(nullptr));
	}
	void CShaderModule_Vulkan::Release()
	{
		if (m_ThisShaderModule != vk::ShaderModule(nullptr))
		{
			GetDevice().destroyShaderModule(m_ThisShaderModule);
			m_ThisShaderModule = nullptr;
		}
		m_OwningProvider = nullptr;
	}
}

