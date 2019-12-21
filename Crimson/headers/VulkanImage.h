#pragma once
#include <headers/VulkanGPUDevice.h>
#include <include/Generals.h>
#include <headers/vk_mem_alloc.h>
#include <vector>

namespace Crimson
{
	class VulkanImageObject : public IGPUImage
	{
	public:
		friend class VulkanGPUDevice;
		VulkanImageObject();
		void SetVulkanImage(VulkanGPUDevice* p_device, VkImage image, VmaAllocation allocation_info,
			EFormat format,
			uint32_t width, 
			uint32_t height, 
			uint32_t depth,
			uint32_t mip_level_num,
			uint32_t layer_num,
			std::vector<EImageUsage> const& usages, EMemoryType memory_type);
		virtual void Dispose() override;
	private:
		VulkanGPUDevice*	p_OwningDevice;
		VkImage				m_Image;
		VmaAllocation		m_Allocation;
	};
}