#include <headers/VulkanPipeline.h>
#include <headers/VulkanDebugLog.h>
#include <vector>

namespace Crimson
{
	VulkanGraphicsPipeline::VulkanGraphicsPipeline(VulkanGPUDevice* device) :
		p_OwningDevice(device)
	{}

	void VulkanGraphicsPipeline::LoadShaderSource(char const* src_code, size_t src_size, EShaderType shader_type)
	{
		VkShaderModule new_shader_module = VK_NULL_HANDLE;
		VkShaderModuleCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.pCode = reinterpret_cast<const uint32_t*>(src_code);
		create_info.codeSize = src_size;
		create_info.flags = 0;
		VulkanDebug::CheckVKResult(vkCreateShaderModule(p_OwningDevice->m_LogicalDevice, &create_info, VULKAN_ALLOCATOR_POINTER, &new_shader_module),
			"Vulkan Creat Shader Module Issue!");
		m_Shaders.push_back(std::make_pair(new_shader_module, shader_type));
	}
	void VulkanGraphicsPipeline::Dispose()
	{
		for (auto& pair : m_Shaders)
		{
			vkDestroyShaderModule(p_OwningDevice->m_LogicalDevice, pair.first, VULKAN_ALLOCATOR_POINTER);
		}
		m_Shaders.clear();
		p_OwningDevice->HandleDisposedGraphicsPipeline(this);
	}
}