#include "private/include/pch.h"
#include "private/include/CShaderModuleObject.h"
#include <private/include/CVulkanApplication.h>

#include "RenderInterface/header/CShaderModule.h"

namespace graphics_backend
{
	CShaderModuleObject::CShaderModuleObject(
		std::shared_ptr<const ShaderProvider> provider
		, CVulkanApplication& application)
	: BaseApplicationSubobject(application)
	, m_Provider(provider)
	{}

	void CShaderModuleObject::Create()
	{
		const std::string codeType = "spirv";
		uint64_t length = m_Provider->GetDataLength(codeType);
		uint32_t codeLength_integer = length / sizeof(uint32_t);
		std::vector<uint32_t> dataArray{codeLength_integer};
		memcpy(dataArray.data(), m_Provider->GetDataPtr(codeType), length);
		vk::ShaderModuleCreateInfo shaderModuelCreateInfo(
			{}
			, dataArray
		);
		std::atomic_thread_fence(std::memory_order_release);
		m_ShaderModule = GetDevice().createShaderModule(shaderModuelCreateInfo);
	}
}

