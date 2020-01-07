#pragma once
#include <headers/VulkanHeader.h>
#include <headers/VulkanGPUDevice.h>
#include <include/Generals.h>
#include <headers/vk_mem_alloc.h>
#include <vector>

namespace Crimson
{
	class VulkanBufferObject : public IGPUBuffer
	{
	public:
		friend class VulkanGPUDevice;
		friend class VulkanGraphicsCommandBuffer;

		VulkanBufferObject();
		void SetVulkanBuffer(VulkanGPUDevice* p_device, VkBuffer buffer, VmaAllocation allocation_info, uint64_t size, std::vector<EBufferUsage> const& usages, EMemoryType memory_type);
		virtual void Dispose() override;
	private:
		VulkanGPUDevice*	p_OwningDevice;
		VkBuffer			m_Buffer;
		VmaAllocation		m_Allocation;
		uint32_t			m_CurrentQueueFamily;
	};
}