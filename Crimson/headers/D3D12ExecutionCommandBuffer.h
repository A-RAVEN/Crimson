#pragma once
#include <include/GPUDevice.h>
#include <include/DescriptorSets.h>
#include <headers/D3D12Header.h>
#include <headers/D3D12GPUDeviceThread.h>
namespace Crimson
{
	class D3D12GPUDevice;
	//class D3D12GPUDeviceThread;
	class D3D12ExecutionCommandBuffer : public ExecutionCommandBuffer
	{
	public:
		friend class D3D12GPUDeviceThread;
		D3D12ExecutionCommandBuffer();
		virtual void ExecuteRenderPassInstance(PRenderPassInstance renderpass_instance) override;
		virtual void CopyBufferToImage(PGPUBuffer buffer, PGPUImage image,
			uint64_t buffer_offset, uint32_t mip_level, uint32_t base_layer, uint32_t layer_count) override {}
		virtual void CopyImageToImage(PGPUImage srd_image, PGPUImage dst_image) override;
		virtual void CopyToSwapchain_Dynamic(PGPUImage image, IWindow* p_window) override;
		virtual void BuildAccelerationStructure(PAccelerationStructure accel_struct,
			PGPUBuffer instance_buffer, uint64_t instance_offset, bool update) override {}
		//virtual void RayTrace(PGPUBuffer shader_table_buffer, uint32_t raygen_id, uint32_t miss_id, uint32_t )
		virtual void BindRayTracer(PRayTracer raytracer) override {}
		virtual void BindRayTracingDescriptorSet(PDescriptorSet descriptor_set, uint32_t set_id) override {}
		virtual void StartRayTracing(PGPUBuffer raygen_table,
			uint64_t raygen_offset, uint64_t miss_offset, uint64_t hit_offset,
			uint32_t width, uint32_t height) override {}
		void TexturesBarrier(std::vector<PGPUImage> const& images) {}
		//no good implementation for all memory in DX12, only resource-wise barrier
		virtual void DeviceMemoryBarrier(EMemoryBarrierType barrier_type) override;
		virtual void BufferBarrier(std::vector<PGPUBuffer> const& buffers, EMemoryBarrierType barrier_type) {};
		virtual void StartCommand() override;
		virtual void EndCommand() override;
		virtual void LoadCache() override {}

		void SetExecutionCommandBuffer(D3D12GPUDevice* p_device, D3D12GPUDeviceThread* p_thread, EExecutionCommandType command_type);
		void Init(D3D12GPUDevice* device, D3D12GPUDeviceThread* thread, CommandAllocatorEntry const& allocator,
			EExecutionCommandType cmd_type, ComPtr<ID3D12GraphicsCommandList6> cmd_list);
	private:
		D3D12GPUDevice* p_OwningDevice;
		D3D12GPUDeviceThread* p_OwningThread;
		CommandAllocatorEntry p_OwningAllocator;
		std::vector<CommandAllocatorEntry*> p_BundleAllocatorReferences;
		ComPtr<ID3D12Fence> p_AllocatprFence;
		HANDLE p_WaitAllocatorEvent;
		//VulkanBatch* p_AttachedBatch;
		ComPtr<ID3D12GraphicsCommandList6> m_CurrentCommandBuffer;
		std::vector<std::pair<ComPtr<ID3D12Fence>, uint64_t>> m_AdditionialWaitingFences;

		//transient container
		std::vector<ComPtr<ID3D12GraphicsCommandList4>> subpassCmdList;
		//std::vector<VkPipelineStageFlags> m_AdditionalWaitingStages;

		//ray tracing
		//VulkanRayTracer* p_CurrentBoundedRayTracer;

		//std::map<VulkanImageObject*, VulkanImageLayoutCache> m_ImageLayoutCaches;
	};
}