#pragma once
#include <headers/VulkanGPUDevice.h>
#include <include/Generals.h>
#include <headers/vk_mem_alloc.h>
#include <vector>
#include <array>

namespace Crimson
{
	class VulkanImageObject : public IGPUImage
	{
	public:
		friend class VulkanGPUDevice;
		friend class VulkanFramebuffer;
		VulkanImageObject();
		void SetVulkanImage(VulkanGPUDevice* p_device, VkImage image, VmaAllocation allocation_info,
			EFormat format,
			uint32_t width, 
			uint32_t height, 
			uint32_t depth,
			uint32_t mip_level_num,
			uint32_t layer_num,
			std::vector<EImageUsage> const& usages, EMemoryType memory_type, VkSharingMode sharing_mode);
		virtual void Dispose() override;
		VkImageSubresourceRange GetFullSubresourceRange(EViewAsType type = EViewAsType::E_VIEW_AS_TYPE_MAX);
		VkImageView GetView(EViewAsType view_as_type);
	private:
		VulkanGPUDevice*	p_OwningDevice;
		VkImage				m_Image;
		VmaAllocation		m_Allocation;
		//Queue family ownership
		uint32_t			m_CurrentQueueFamily;
		uint32_t			m_NextQueueFamily;
		//Overall image layout, assume that all the mip levels and layers have same layout (which may not be true)
		VkImageLayout		m_OverallImageLayout;
		VkSharingMode		m_SharingMode;
		VkAccessFlags		m_CurrentAccessMask;
		std::array<int8_t, static_cast<size_t>(EViewAsType::E_VIEW_AS_TYPE_MAX)> m_ImageViewMap;
		std::vector<VkImageView> m_ImageViews;
		EViewAsType m_DefaultViewAsType;
	};
}