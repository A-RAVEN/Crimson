#pragma once
#include <include/Pipeline.h>
#include <headers/D3D12GPUDevice.h>
#include <headers/HelperContainers.h>

namespace Crimson
{
	struct PipelineStateStream
	{
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
		CD3DX12_PIPELINE_STATE_STREAM_VS VS;
		CD3DX12_PIPELINE_STATE_STREAM_PS PS;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
	} pipelineStateStream;
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
		void BuildPipeline();
	private:
		D3D12GPUDevice* p_OwningDevice;
		ByteVector m_PipelineStateStreamingData;
		std::vector<D3D12_INPUT_ELEMENT_DESC> m_VertexInputDescriptors;
		//std::vector<std::pair<VkShaderModule, EShaderType>>	m_Shaders;
		//std::vector<VulkanShaderModule*> m_ShaderModules;
	};
}