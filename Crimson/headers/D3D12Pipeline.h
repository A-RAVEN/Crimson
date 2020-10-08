#pragma once
#include <include/Pipeline.h>
#include <headers/D3D12GPUDevice.h>
#include <headers/HelperContainers.h>

namespace Crimson
{
	//class VulkanShaderModule;
	class D3D12GraphicsPipeline : public GraphicsPipeline
	{
	public:
		//friend class VulkanRenderPass;
		//friend class VulkanGraphicsCommandBuffer;

		D3D12GraphicsPipeline(D3D12GPUDevice* device);
		virtual void LoadShaderSource(char const* src_code, size_t src_size, EShaderType shader_type) override;
		virtual void LoadShaderModule(PShaderModule shader_module) override;
		virtual void Dispose() override;
		virtual void BuildPipeline() override;
	private:
		D3D12GPUDevice* p_OwningDevice;
		ByteVector m_PipelineStateStreamingData;
		std::vector<D3D12_INPUT_ELEMENT_DESC> m_VertexInputDescriptors;
		ComPtr<ID3D12RootSignature> m_RootSignature;
		//std::vector<std::pair<VkShaderModule, EShaderType>>	m_Shaders;
		//std::vector<VulkanShaderModule*> m_ShaderModules;
	};
}