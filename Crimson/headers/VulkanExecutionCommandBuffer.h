#pragma once
#include <include/GPUDevice.h>
#include <headers/VulkanHeader.h>
#include <headers/VulkanGPUDevice.h>
#include <headers/VulkanGPUDeviceThread.h>
#include <headers/VulkanBatch.h>

namespace Crimson
{
	class VulkanExecutionCommandBuffer : public ExecutionCommandBuffer
	{
	public:
		friend class VulkanGPUDeviceThread;
		VulkanExecutionCommandBuffer();
		virtual void ExecuteRenderPassInstance(PRenderPassInstance renderpass_instance) override;
		virtual void CopyToSwapchain(PGPUImage image, IWindow* p_window) override;
		virtual void StartCommand() override;
		virtual void EndCommand() override;
		void SetExecutionCommandBuffer(VulkanGPUDevice* p_device, VulkanGPUDeviceThread* p_thread, EExecutionCommandType command_type);
	private:
		VulkanGPUDevice* p_OwningDevice;
		VulkanGPUDeviceThread* p_OwningThread;
		VulkanBatch* p_AttachedBatch;
		EExecutionCommandType m_CommandType;
		VkCommandBuffer m_CurrentCommandBuffer;
	};
}