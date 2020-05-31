#include <headers/VulkanShaderModule.h>

namespace Crimson
{
	void Crimson::VulkanShaderModule::Init(VulkanGPUDevice* p_device, VkShaderModule shader_module)
	{
		m_Device = p_device;
		m_ShaderModule = shader_module;
	}

	void Crimson::VulkanShaderModule::Dispose()
	{
		m_Device->HandleDIsposedShaderModule(this);
	}
}