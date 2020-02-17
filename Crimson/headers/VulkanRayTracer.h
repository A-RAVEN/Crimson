#pragma once
#include <include/RayTracer.h>
#include <headers/VulkanGPUDevice.h>

namespace Crimson
{
	class VulkanRayTracer : public RayTracer
	{
	public:
		friend class VulkanRenderPass;
		friend class VulkanGraphicsCommandBuffer;
		friend class VulkanExecutionCommandBuffer;

		VulkanRayTracer(VulkanGPUDevice* device);
		virtual void LoadShaderSource(char const* src_code, size_t src_size, EShaderType shader_type, std::string const& shader_table) override;
		virtual uint64_t GetShaderTableSize(std::string const& shader_table_name) override;
		virtual void CopyShaderTable(void* copy_dst, std::string const& shader_table_name) override;
		virtual void Build() override;
		virtual void Dispose() override;
	private:
		void DestroyPipeline();
		void DestroyShaderModules();
		VulkanGPUDevice* p_OwningDevice;
		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_Pipeline;
		std::vector<std::pair<VkShaderModule, EShaderType>>	m_Shaders;

		std::map<std::string, std::vector<size_t>> m_ShaderTables;
		std::vector<char> m_Handles;
	};
}