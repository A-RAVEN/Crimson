#pragma once
#include <include/GPUDevice.h>
#include <headers/D3D12Header.h>
#include <deque>
#include <set>
#include <unordered_map>
#include <queue>

namespace Crimson
{
	class D3D12ExecutionCommandBuffer;
	class D3D12GPUDevice;
	struct CommandAllocatorEntry
	{
		ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
		D3D12_COMMAND_LIST_TYPE m_CmdQueueType;
		uint32_t m_SubmitQueue;
		uint64_t m_SubmitSignalValue;
	};

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
		virtual void BindExecutionCommandBufferToBatch(std::string const& batch_name, PExecutionCommandBuffer command_buffer, bool one_time) override;

		void InitGPUDeviceThread(D3D12GPUDevice* device);

		ComPtr<ID3D12GraphicsCommandList4> GetSubpassCommandList(PRenderPassInstance instance, std::vector<CommandAllocatorEntry*>& allocators, uint32_t subpassId);

		//accessed by execution command buffer
		ComPtr<ID3D12GraphicsCommandList6> AllocExecutionD3D12CommandList(EExecutionCommandType cmd_type, CommandAllocatorEntry& initialAllocator);
		void RecycleExecutionD3D12CommandList(ComPtr<ID3D12GraphicsCommandList6> cmd_buffer, EExecutionCommandType cmd_type);
		struct ThreadBatchData
		{
			std::vector<D3D12ExecutionCommandBuffer*> m_GraphicsExecutionBuffers;
			std::vector<D3D12ExecutionCommandBuffer*> m_ComputeExecutionBuffers;
			std::vector<D3D12ExecutionCommandBuffer*> m_CopyExecutionBuffers;
			void CollectCmdLists(EExecutionCommandType cmdType, std::vector<ID3D12CommandList *>& list, std::vector<std::pair<ComPtr<ID3D12Fence>, uint64_t>>& allocator_fences, uint32_t queue_id, uint64_t signal_val);
		};
		CommandAllocatorEntry AllocCommandAllocator(D3D12_COMMAND_LIST_TYPE type);
		void ReturnCommandAllocator(CommandAllocatorEntry& return_entry);
	private:

		std::queue<CommandAllocatorEntry> m_CommandAllocatorQueues[7];

		void InitCommandAllocators();

		D3D12GPUDevice* p_OwningDevice;

		ComPtr<ID3D12CommandAllocator> m_GraphicsCommandPool;
		ComPtr<ID3D12CommandAllocator> m_ComputeCommandPool;
		ComPtr<ID3D12CommandAllocator> m_CopyCommandPool;
		std::vector<ThreadBatchData> m_BatchDataList;

		std::unordered_map < PRenderPassInstance, std::vector<std::pair<ComPtr<ID3D12GraphicsCommandList4>, CommandAllocatorEntry>> > m_SubpassCommandLists;

	};
}