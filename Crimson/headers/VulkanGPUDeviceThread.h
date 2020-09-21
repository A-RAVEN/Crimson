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
	class VulkanDescriptorSet;

	struct SubpassCommandInfo
	{
		VkCommandBuffer m_CommandBuffer;
		std::vector<VulkanDescriptorSet*> m_ReferencingDescriptorSets;
		SubpassCommandInfo() :
			m_CommandBuffer(VK_NULL_HANDLE),
			m_ReferencingDescriptorSets()
		{}
	};

	class RenderPassInstanceGraphicsCommandBufferInfo
	{
	public:
		RenderPassInstanceGraphicsCommandBufferInfo();
		void InitBufferInfo(VulkanRenderPassInstance* p_instance);
		//std::vector<VkCommandBuffer> m_SubpassCommands;
		std::vector<SubpassCommandInfo> m_SubpassCommands;
	};

	class VulkanGPUDevice;
	class VulkanGPUDeviceThread : public IGPUDeviceThread
	{
	public:
		friend class VulkanGPUDevice;
		friend class VulkanGraphicsCommandBuffer;
		VulkanGPUDeviceThread();
		~VulkanGPUDeviceThread();
		virtual void Dispose() override;
		virtual PGraphicsCommandBuffer StartSubpassCommand(PRenderPassInstance renderpass_instance, uint32_t subpass_id) override;
		void HandleDisposedGraphicsCommandBuffer(VulkanGraphicsCommandBuffer* cmd_buffer);
		virtual PExecutionCommandBuffer CreateExecutionCommandBuffer(EExecutionCommandType cmd_type) override;
		virtual void BindExecutionCommandBufferToBatch(std::string const& batch_name, PExecutionCommandBuffer command_buffer, bool one_time) override;
		
		void OnGraphicsCommandBufferFinished(VulkanGraphicsCommandBuffer* cmd_buffer);
		void InitGPUDeviceThread(VulkanGPUDevice* device);
		void PushBackSubpassCommandBuffer(std::vector<VkCommandBuffer>& cmd_buffers, uint32_t renderpass_instance_id, uint32_t subpass_id, std::deque<VulkanDescriptorSet*>& referenced_sets);
		void PushBackExecutionCommandBuffers(std::vector<VkCommandBuffer>& cmd_buffers, uint32_t batch_unique_id, std::vector<VkSemaphore>& waiting_semaphores, std::vector<VkPipelineStageFlags>& waiting_stages);

		//accessed by execution command buffer
		VkCommandBuffer AllocExecutionVkCommandBuffer(EExecutionCommandType cmd_type);
		void RecycleExecutionVkCommandBuffer(VkCommandBuffer cmd_buffer, EExecutionCommandType cmd_type);
	private:
		void InitGraphicsCommandPool();
		void InitComputeCommandPool();

		VulkanGPUDevice* p_OwningDevice;

		VkCommandPool m_GraphicsCommandPool;
		VkCommandPool m_ComputeCommandPool;

		//obsolete
		std::deque<VkCommandBuffer> m_RecycledGraphicsCommandBuffer;

		std::deque<VkCommandBuffer> m_RecycledGraphicsExecutionCommandBuffer;
		std::deque<VkCommandBuffer> m_RecycledComputeExecutionCommandBuffer;

		std::deque<uint32_t> m_RenderPassInstanceGraphicsCommandBufferInfoReferences;
		std::deque<RenderPassInstanceGraphicsCommandBufferInfo> m_RenderPassInstanceGraphicsCommandBufferInfos;

		std::deque<
			std::pair<std::set<VulkanExecutionCommandBuffer*>, std::set<VulkanExecutionCommandBuffer*>>
		> m_AttachedExecutionCommandBuffers;
	};
}