#pragma once
#include <headers/D3D12GPUDevice.h>
#include <headers/D3D12RenderPass.h>
#include <headers/D3D12Framebuffer.h>
#include <include/Generals.h>

namespace Crimson
{
	class D3D12RenderPassInstance : public RenderPassInstance
	{
	public:
		friend class D3D12GPUDeviceThread;
		friend class D3D12GraphicsCommandBuffer;
		virtual void Call() = 0;
		void Init(D3D12GPUDevice* owning_device, D3D12RenderPass* renderpass, D3D12Framebuffer* framebuffer);
		struct SubpassInstanceInfo
		{
		public:
			std::vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC> m_RenderTargetDescriptors;
			D3D12_RENDER_PASS_DEPTH_STENCIL_DESC m_DepthStencilDescriptor;
			bool b_HasDepthStencil = false;
		};
	private:
		D3D12GPUDevice* p_OwningDevice;
		std::vector<SubpassInstanceInfo> m_SubpassInstances;
		D3D12RenderPass* p_DXRenderPass;
		D3D12Framebuffer* p_DXFramebuffer;
	};
}