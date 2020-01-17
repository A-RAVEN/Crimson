#pragma once
#include <include/GPUDevice.h>
#include <headers/VulkanHeader.h>
#include <deque>
#include <set>

namespace Crimson
{
	class VulkanGraphicsCommandBuffer;
	class VulkanRenderPassInstance;
	class VulkanBatch;
	class VulkanExecutionCommandBuffer;
	class RenderPassInstanceGraphicsCommandBufferInfo
	{
	public:
		RenderPassInstanceGraphicsCommandBufferInfo();
		void InitBufferInfo(VulkanRenderPassInstance* p_instance);
		std::vector<VkCommandBuffer> m_SubpassCommands;
	};

	class VulkanGPUDevice;
	class VulkanGPUDeviceThread : public IGPUDeviceThread
	{
	public:
		friend class VulkanGPUDevice;
		VulkanGPUDeviceThread();
		~VulkanGPUDeviceThread();
		virtual void Dispose() override;
		virtual PGraphicsCommandBuffer StartSubpassCommand(PRenderPassInstance renderpass_instance, uint32_t subpass_id) override;
		virtual PExecutionCommandBuffer CreateExecutionCommandBuffer(EExecutionCommandType cmd_type) override;
		virtual void BindExecutionCommandBufferToBatch(std::string const& batch_name, PExecutionCommandBuffer command_buffer);
		void OnGraphicsCommandBufferFinished(VulkanGraphicsCommandBuffer* cmd_buffer);
		void InitGPUDeviceThread(VulkanGPUDevice* device);
		void PushBackSubpassCommandBuffer(std::vector<VkCommandBuffer>& cmd_buffers, uint32_t renderpass_instance_id, uint32_t subpass_id);
	private:
		void InitGraphicsCommandPool();
		void InitComputeCommandPool();
		VkCommandBuffer AllocExecutionVkCommandBuffer(EExecutionCommandType cmd_type);
		void RecycleExecutionVkCommandBuffer(VkCommandBuffer cmd_buffer, EExecutionCommandType cmd_type);

		VulkanGPUDevice* p_OwningDevice;

		VkCommandPool m_GraphicsCommandPool;
		VkCommandPool m_ComputeCommandPool;

		std::deque<VkCommandBuffer> m_RecycledGraphicsCommandBuffer;

		std::deque<VkCommandBuffer> m_RecycledGraphicsExecutionCommandBuffer;
		std::deque<VkCommandBuffer> m_RecycledComputeExecutionCommandBuffer;

		std::deque<uint32_t> m_RenderPassInstanceGraphicsCommandBufferInfoReferences;
		std::deque<RenderPassInstanceGraphicsCommandBufferInfo> m_RenderPassInstanceGraphicsCommandBufferInfos;

		std::deque<std::set<VulkanExecutionCommandBuffer*>> m_AttachedExecutionCommandBuffers;
	};
}