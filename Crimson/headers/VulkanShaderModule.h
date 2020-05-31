#pragma once
#include <include/Pipeline.h>
#include <headers/VulkanGPUDevice.h>

namespace Crimson
{
	class VulkanShaderModule : public ShaderModule
	{
	public:
		friend class VulkanGPUDevice;
		void Init(VulkanGPUDevice* p_device, VkShaderModule shader_module);
		virtual void Dispose() override;
	private:
		VulkanGPUDevice* m_Device;
		VkShaderModule m_ShaderModule;
	};
}