#pragma once
#include <include/GPUDevice.h>
#include <headers/VulkanHeader.h>
#include <headers/VulkanGPUDevice.h>
#include <headers/VulkanGPUDeviceThread.h>

namespace Crimson
{
	class VulkanExecutionCommandBuffer : public ExecutionCommandBuffer
	{
	public:
		virtual void ExecuteRenderPassInstance(PRenderPassInstance renderpass_instance) override;

	private:
		VulkanGPUDeviceThread* p_OwningThread;
		uint32_t m_QueueFamilyId;
		VkCommandBuffer m_CurrentCommandBuffer;
	};
}