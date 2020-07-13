#pragma once
#include <include/GPUDevice.h>
#include <headers/D3D12Header.h>
#include <deque>
#include <set>

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
		//void HandleDisposedGraphicsCommandBuffer(VulkanGraphicsCommandBuffer* cmd_buffer);
		virtual PExecutionCommandBuffer CreateExecutionCommandBuffer(EExecutionCommandType cmd_type) override;
		virtual void BindExecutionCommandBufferToBatch(std::string const& batch_name, PExecutionCommandBuffer command_buffer, bool one_time) override;

		//void OnGraphicsCommandBufferFinished(VulkanGraphicsCommandBuffer* cmd_buffer);
		void InitGPUDeviceThread(D3D12GPUDevice* device);
		//void PushBackSubpassCommandBuffer(std::vector<VkCommandBuffer>& cmd_buffers, uint32_t renderpass_instance_id, uint32_t subpass_id, std::deque<VulkanDescriptorSet*>& referenced_sets);
		//void PushBackExecutionCommandBuffers(std::vector<VkCommandBuffer>& cmd_buffers, uint32_t batch_unique_id, std::vector<VkSemaphore>& waiting_semaphores, std::vector<VkPipelineStageFlags>& waiting_stages);

		//VkCommandBuffer AllocExecutionVkCommandBuffer(EExecutionCommandType cmd_type);
		//void RecycleExecutionVkCommandBuffer(VkCommandBuffer cmd_buffer, EExecutionCommandType cmd_type);
	private:
		void InitCommandAllocators();

		D3D12GPUDevice* p_OwningDevice;

		ComPtr<ID3D12CommandAllocator> m_GraphicsCommandPool;
		ComPtr<ID3D12CommandAllocator> m_ComputeCommandPool;
		ComPtr<ID3D12CommandAllocator> m_CopyCommandPool;

		//std::deque<uint32_t> m_RenderPassInstanceGraphicsCommandBufferInfoReferences;
		//std::deque<RenderPassInstanceGraphicsCommandBufferInfo> m_RenderPassInstanceGraphicsCommandBufferInfos;

		//std::deque<
		//	std::pair<std::set<VulkanExecutionCommandBuffer*>, std::set<VulkanExecutionCommandBuffer*>>
		//> m_AttachedExecutionCommandBuffers;
	};
}