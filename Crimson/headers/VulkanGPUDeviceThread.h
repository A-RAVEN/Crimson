#pragma once
#include <include/GPUDevice.h>
#include <headers/VulkanHeader.h>
#include <deque>

namespace Crimson
{
	class VulkanGraphicsCommandBuffer;
	class VulkanRenderPassInstance;
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
		void OnGraphicsCommandBufferFinished(VulkanGraphicsCommandBuffer* cmd_buffer);
		void InitGPUDeviceThread(VulkanGPUDevice* device);
	private:
		void InitGraphicsCommandPool();
		void InitComputeCommandPool();
		VulkanGPUDevice* p_OwningDevice;
		VkCommandPool m_GraphicsCommandPool;
		VkCommandPool m_ComputeCommandPool;

		std::deque<VkCommandBuffer> m_RecycledGraphicsCommandBuffer;
		std::deque<uint32_t> m_RenderPassInstanceGraphicsCommandBufferInfoReferences;
		std::deque<RenderPassInstanceGraphicsCommandBufferInfo> m_RenderPassInstanceGraphicsCommandBufferInfos;
	};
}