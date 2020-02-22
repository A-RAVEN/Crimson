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
		friend class VulkanExecutionCommandBuffer;
		friend class VulkanDescriptorSet;

		VulkanBufferObject();
		void SetVulkanBuffer(VulkanGPUDevice* p_device, VkBuffer buffer, VmaAllocation allocation_info, uint64_t size, std::vector<EBufferUsage> const& usages, EMemoryType memory_type);
		virtual void Dispose() override;
		virtual uint8_t* GetMappedPointer() override;
		virtual void UnMapp() override;
		inline VkBuffer GetVulkanBuffer() const { return m_Buffer; }

	private:
		VulkanGPUDevice*	p_OwningDevice;
		VkBuffer			m_Buffer;
		VmaAllocation		m_Allocation;
		uint32_t			m_CurrentQueueFamily;
		uint8_t*			p_Mapped;
	};
}