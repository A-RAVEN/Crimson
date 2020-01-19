#pragma once
#include <headers/VulkanGPUDevice.h>

namespace Crimson {
	class VulkanBatch
	{
	public:
		void SetBatch(VulkanGPUDevice* p_owning_device, uint32_t batch_id, uint32_t queue_family_id, uint32_t priority);
		void SubmitCommands();
		void DestroyBatch();
		void Wait();
		VulkanGPUDevice*	p_OwningDevice;
		VkFence				m_Fence;
		VkSemaphore			m_Semaphore;
		VkQueue				m_Queue;
		uint32_t			m_BatchID;
		uint32_t			m_QueueFamily;
		uint32_t			m_QueuePriority;
	};
}