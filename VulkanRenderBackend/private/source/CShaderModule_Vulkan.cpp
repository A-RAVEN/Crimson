#include "private/include/pch.h"
#include "private/include/CShaderModuleObject.h"
#include <private/include/CVulkanApplication.h>

#include "RenderInterface/header/CShaderModule.h"

namespace graphics_backend
{
	CShaderModuleObject::CShaderModuleObject(CVulkanApplication& application)
	: BaseApplicationSubobject(application)
	{}

	void CShaderModuleObject::Create(ShaderModuleDescritor const& descriptor)
	{
		const std::string codeType = "spirv";
		uint64_t length = descriptor.provider->GetDataLength(codeType);
		uint32_t codeLength_integer = length / sizeof(uint32_t);
		std::vector<uint32_t> dataArray;
		dataArray.resize(codeLength_integer);
		memcpy(dataArray.data(), descriptor.provider->GetDataPtr(codeType), length);
		vk::ShaderModuleCreateInfo shaderModuelCreateInfo(
			{}
			, dataArray
		);
		std::atomic_thread_fence(std::memory_order_release);
		m_ShaderModule = GetDevice().createShaderModule(shaderModuelCreateInfo);
	}
	void CShaderModuleObject::Release()
	{
		if (m_ShaderModule != vk::ShaderModule(nullptr))
		{
			GetDevice().destroyShaderModule(m_ShaderModule);
			m_ShaderModule = nullptr;
		}
	}
}

