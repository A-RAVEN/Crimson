#pragma once
#include <include/GPUDevice.h>
#include <headers/VulkanHeader.h>

namespace Crimson
{
	class VulkanGPUDevice;
	class VulkanGPUDeviceThread : public IGPUDeviceThread
	{
	public:
		friend class VulkanGPUDevice;
		virtual PGraphicsCommandBuffer StartSubpassCommand(PRenderPassInstance renderpass_instance, uint32_t subpass_id);
		void InitGPUDeviceThread(VulkanGPUDevice* device);
	private:
		void InitGraphicsCommandPool();
		void InitComputeCommandPool();
		VulkanGPUDevice* p_OwningDevice;
		VkCommandPool m_GraphicsCommandPool;
		VkCommandPool m_ComputeCommandPool;
	};
}