#pragma once
#include <include/GPUDevice.h>
#include <headers/D3D12Header.h>
#include <deque>
#include <set>
#include <unordered_map>

namespace Crimson
{

	class D3D12GPUDevice;
	class D3D12GPUDeviceThread : public IGPUDeviceThread
	{
	public:
		friend class D3D12GPUDevice;
		//friend class VulkanGraphicsCommandBuffer;
		D3D12GPUDeviceThread();
		~D3D12GPUDeviceThread();
		virtual void Dispose() override;
		virtual PGraphicsCommandBuffer StartSubpassCommand(PRenderPassInstance renderpass_instance, uint32_t subpass_id) override;

		virtual PExecutionCommandBuffer CreateExecutionCommandBuffer(EExecutionCommandType cmd_type) override;
		virtual void BindExecutionCommandBufferToBatch(std::string const& batch_name, PExecutionCommandBuffer command_buffer, bool one_time) override {}

		void InitGPUDeviceThread(D3D12GPUDevice* device);

		ComPtr<ID3D12GraphicsCommandList4> GetSubpassCommandList(PRenderPassInstance instance, uint32_t subpassId);

		//accessed by execution command buffer
		ComPtr<ID3D12GraphicsCommandList6> AllocExecutionD3D12CommandList(EExecutionCommandType cmd_type, ComPtr<ID3D12CommandAllocator>& ownerAllocator);
		void RecycleExecutionD3D12CommandList(ComPtr<ID3D12GraphicsCommandList6> cmd_buffer, EExecutionCommandType cmd_type);
	private:
		void InitCommandAllocators();

		D3D12GPUDevice* p_OwningDevice;

		ComPtr<ID3D12CommandAllocator> m_GraphicsCommandPool;
		ComPtr<ID3D12CommandAllocator> m_ComputeCommandPool;
		ComPtr<ID3D12CommandAllocator> m_CopyCommandPool;

		std::unordered_map < PRenderPassInstance, std::vector<ComPtr<ID3D12GraphicsCommandList4>> > m_SubpassCommandLists;
		//std::deque<uint32_t> m_RenderPassInstanceGraphicsCommandBufferInfoReferences;
		//std::deque<RenderPassInstanceGraphicsCommandBufferInfo> m_RenderPassInstanceGraphicsCommandBufferInfos;

		//std::deque<
		//	std::pair<std::set<VulkanExecutionCommandBuffer*>, std::set<VulkanExecutionCommandBuffer*>>
		//> m_AttachedExecutionCommandBuffers;
	};
}