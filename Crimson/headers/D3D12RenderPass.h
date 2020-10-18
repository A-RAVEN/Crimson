#pragma once
#include <include/Generals.h>
#include <headers/D3D12GPUDevice.h>
#include <headers/HelperContainers.h>
#include <unordered_map>

namespace Crimson
{
	class D3D12RenderPass : public RenderPass
	{
	public:
		friend class D3D12RenderPassInstance;
		friend class D3D12GraphicsCommandBuffer;
		virtual void BuildRenderPass();
		virtual void InstanciatePipeline(GraphicsPipeline* pipeline, uint32_t subpass);
		virtual void Dispose() override;
		void InitRenderPass(D3D12GPUDevice* p_device);
		struct SubpassInfo
		{
			std::vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC> m_RenderTargetDescriptors;
			D3D12_RENDER_PASS_DEPTH_STENCIL_DESC m_DepthStencilDescriptor;
			ByteVector m_PipelineStateStreamingDataRenderTargetFormats;
			std::unordered_map<PGraphicsPipeline, ComPtr<ID3D12PipelineState>> pipelineInstances;
		};
	private:
		///					isDS	id
		std::vector<std::pair<bool, uint32_t>> m_RenderTargets;
		std::vector< SubpassInfo> m_D3D12Subpasses;
		D3D12GPUDevice* p_OwningDevice;
	};
}