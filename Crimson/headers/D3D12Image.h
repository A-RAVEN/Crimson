#pragma once
#include <headers/D3D12GPUDevice.h>
#include <include/Generals.h>

namespace Crimson
{
	class D3D12ImageObject : public IGPUImage
	{
	public:
		friend class D3D12GPUDevice;
		friend class D3D12Framebuffer;
		//friend class VulkanExecutionCommandBuffer;
		//friend class VulkanDescriptorSet;
		D3D12ImageObject();
		void SetD3D12Image(D3D12GPUDevice* p_device,
			EFormat format,
			uint32_t width,
			uint32_t height,
			uint32_t depth,
			uint32_t mip_level_num,
			uint32_t layer_num,
			std::vector<EImageUsage> const& usages, EMemoryType memory_type);
		virtual void Dispose() override;
		//VkImageSubresourceRange GetFullSubresourceRange(EViewAsType type = EViewAsType::E_VIEW_AS_TYPE_MAX) const;
		//VkImageSubresourceLayers GetFullSubresourceLayers(EViewAsType type = EViewAsType::E_VIEW_AS_TYPE_MAX, uint32_t mip_level = 0) const;
		//VkImageSubresourceLayers GetSubresourceLayers(EViewAsType type = EViewAsType::E_VIEW_AS_TYPE_MAX, uint32_t mip_level = 0, uint32_t base_layer = 0, uint32_t layer_num = 1) const;
		//VkImageView GetView(EViewAsType view_as_type);
		//TODO: Improve to a more efficient way
		//VkSampler GetSampler(EFilterMode filter_mode, EAddrMode address_mode);
		//EViewAsType GetDefaultViewAsType() { return m_DefaultViewAsType; }
		//void CmdChangeOverallLayout(VkCommandBuffer cmd_buffer, uint32_t queue_family, VkImageLayout dst_layout, VkPipelineStageFlags dst_stage, VkPipelineStageFlags finished_stage);
		//VulkanImageLayoutCache GetCurrentLayoutCache() const;
		//void ApplyLayoutCache(uint32_t queue_family, VulkanImageLayoutCache const& cache);
	private:
		D3D12GPUDevice* p_OwningDevice;
		ComPtr<ID3D12Resource> m_Image;
		//Queue family ownership
		uint32_t			m_CurrentQueueFamily;
		uint32_t			m_NextQueueFamily;
		//Overall image layout, assume that all the mip levels and layers have same layout (which may not be true)
		//std::array<int8_t, static_cast<size_t>(EViewAsType::E_VIEW_AS_TYPE_MAX)> m_ImageViewMap;
		//std::vector<VkImageView> m_ImageViews;
		//EViewAsType m_DefaultViewAsType;

		//std::vector<VulkanImageSubresourceData> m_SubresourceData;

		//std::array<std::array<VkSampler, static_cast<size_t>(EAddrMode::E_ADDR_MAX)>,
			//static_cast<size_t>(EFilterMode::E_FILTER_MODE_MAX)> m_Samplers;
	};
}