#pragma once
#include <include/Pipeline.h>
#include <headers/D3D12GPUDevice.h>
#include <headers/HelperContainers.h>
#include <deque>

namespace Crimson
{
	struct PipelineStreamStruct
	{
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY	topology;
		CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT			inputLayout;
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE		rootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC			blend;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL			depthStencil;
		CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER			rasterizer;
		CD3DX12_PIPELINE_STATE_STREAM_PS					pixelShader;
		CD3DX12_PIPELINE_STATE_STREAM_VS					vertexShader;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS	rtFormats;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT	dsFormat;
	};
	//class VulkanShaderModule;
	class D3D12GraphicsPipeline : public GraphicsPipeline
	{
	public:
		friend class D3D12RenderPass;
		friend class D3D12GraphicsCommandBuffer;

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
		PipelineStreamStruct m_Stream;
		std::deque<std::string> m_FullSematics;
		D3D_PRIMITIVE_TOPOLOGY m_D3DPrimitiveTopology;
	};
}