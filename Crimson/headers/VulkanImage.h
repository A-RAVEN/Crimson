#pragma once
#include <headers/VulkanGPUDevice.h>
#include <include/Generals.h>
#include <headers/vk_mem_alloc.h>
#include <vector>
#include <array>

namespace Crimson
{
	class VulkanImageTransitionData
	{
	public:
		VulkanImageObject* p_image;

	};

	class VulkanImageObject : public IGPUImage
	{
	public:
		friend class VulkanGPUDevice;
		friend class VulkanFramebuffer;
		friend class VulkanExecutionCommandBuffer;
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
		VkImageSubresourceRange GetFullSubresourceRange(EViewAsType type = EViewAsType::E_VIEW_AS_TYPE_MAX) const;
		VkImageSubresourceLayers GetFullSubresourceLayers(EViewAsType type = EViewAsType::E_VIEW_AS_TYPE_MAX) const;
		VkImageView GetView(EViewAsType view_as_type);
		//TODO: Improve to a more efficient way
		VkSampler GetSampler(EFilterMode filter_mode, EAddrMode address_mode);
		EViewAsType GetDefaultViewAsType() { return m_DefaultViewAsType; }
		void CmdChangeOverallLayout(VkCommandBuffer cmd_buffer, uint32_t queue_family, VkImageLayout dst_layout, VkPipelineStageFlags dst_stage, VkPipelineStageFlags finished_stage);
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
		VkPipelineStageFlags m_LastUsingStage;
		std::array<int8_t, static_cast<size_t>(EViewAsType::E_VIEW_AS_TYPE_MAX)> m_ImageViewMap;
		std::vector<VkImageView> m_ImageViews;
		EViewAsType m_DefaultViewAsType;

		std::array<std::array<VkSampler, static_cast<size_t>(EAddrMode::E_ADDR_MAX)>,
			static_cast<size_t>(EFilterMode::E_FILTER_MODE_MAX)> m_Samplers;
	};
}