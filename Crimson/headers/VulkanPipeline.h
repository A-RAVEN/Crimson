#pragma once
#include <include/Pipeline.h>
#include <headers/VulkanGPUDevice.h>

namespace Crimson
{
	class VulkanGraphicsPipeline : public GraphicsPipeline
	{
	public:
		friend class VulkanRenderPass;
		friend class VulkanGraphicsCommandBuffer;

		VulkanGraphicsPipeline(VulkanGPUDevice* device);
		virtual void LoadShaderSource(char const* src_code, size_t src_size, EShaderType shader_type) override;
		virtual void Dispose() override;
	private:
		VulkanGPUDevice* p_OwningDevice;
		std::vector<std::pair<VkShaderModule, EShaderType>>	m_Shaders;
	};
}