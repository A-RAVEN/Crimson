#include <headers/VulkanImage.h>
#include <headers/VulkanTranslator.h>
#include <headers/VulkanDebugLog.h>
#include <headers/GeneralDebug.h>

namespace Crimson
{
	VulkanImageObject::VulkanImageObject() : IGPUImage(),
		p_OwningDevice(nullptr),
		m_Image(VK_NULL_HANDLE),
		m_Allocation(nullptr),
		m_CurrentQueueFamily(VK_QUEUE_FAMILY_IGNORED),
		m_NextQueueFamily(VK_QUEUE_FAMILY_IGNORED),
		m_OverallImageLayout(VK_IMAGE_LAYOUT_UNDEFINED),
		m_SharingMode(VK_SHARING_MODE_EXCLUSIVE),
		m_CurrentAccessMask(0),
		m_ImageViewMap(),
		m_DefaultViewAsType(EViewAsType::E_VIEW_AS_TYPE_MAX),
		m_LastUsingStage(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT)
	{
		std::fill(m_ImageViewMap.begin(), m_ImageViewMap.end(), int8_t(-1));
		for (auto& itr_vec : m_Samplers)
		{
			for (auto& itr_samp : itr_vec)
			{
				itr_samp = VK_NULL_HANDLE;
			}
		}
	}
	void VulkanImageObject::SetVulkanImage(VulkanGPUDevice* p_device, 
		VkImage image, 
		VmaAllocation allocation_info, 
		EFormat format, uint32_t width, 
		uint32_t height, uint32_t depth, 
		uint32_t mip_level_num, 
		uint32_t layer_num, 
		std::vector<EImageUsage> const& usages, 
		EMemoryType memory_type, 
		VkSharingMode sharing_mode)
	{
		p_OwningDevice = p_device;
		m_Image = image;
		m_Allocation = allocation_info;
		m_Format = format;
		m_Width = width;
		m_Height = height;
		m_Depth = depth;
		m_MipLevelNum = mip_level_num;
		m_LayerNum = layer_num;
		m_ImageUsages = usages;
		m_MemoryType = memory_type;
		m_OverallImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		m_SharingMode = sharing_mode;
		if (IsColorFormat(m_Format))
		{
			m_DefaultViewAsType = EViewAsType::E_VIEW_AS_COLOR;
		}
		else if (IsStencilOnlyFormat(m_Format))
		{
			m_DefaultViewAsType = EViewAsType::E_VIEW_AS_STENCIL;
		}
		else if (IsDepthOnlyFormat(m_Format))
		{
			m_DefaultViewAsType = EViewAsType::E_VIEW_AS_DEPTH;
		}
		else
		{
			m_DefaultViewAsType = EViewAsType::E_VIEW_AS_DEPTH_STENCIL;
		}
	}
	void VulkanImageObject::Dispose()
	{
		for (auto view : m_ImageViews)
		{
			vkDestroyImageView(p_OwningDevice->m_LogicalDevice, view, VULKAN_ALLOCATOR_POINTER);
		}
		m_ImageViews.clear();
		std::fill(m_ImageViewMap.begin(), m_ImageViewMap.end(), int8_t(-1));
		p_OwningDevice->HandleDisposedImage(this);
	}
	VkImageSubresourceRange VulkanImageObject::GetFullSubresourceRange(EViewAsType type) const
	{
		VkImageSubresourceRange return_val{};
		return_val.baseArrayLayer = 0;
		return_val.baseMipLevel = 0;
		return_val.layerCount = m_LayerNum;
		return_val.levelCount = m_MipLevelNum;
		if (type == EViewAsType::E_VIEW_AS_TYPE_MAX)
		{
			return_val.aspectMask = TranslateViewAsTypeToVulkanAspectFlags(m_DefaultViewAsType);
		}
		else
		{
			return_val.aspectMask = TranslateViewAsTypeToVulkanAspectFlags(type);
		}
	/*	if (IsColorFormat(m_Format))
		{
			return_val.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}
		else
		{
			if (IsDepthOnlyFormat(m_Format))
			{
				return_val.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			}
			else if (IsStencilOnlyFormat(m_Format))
			{
				return_val.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
			}
			else
			{
				return_val.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}*/
		return return_val;
	}
	VkImageSubresourceLayers VulkanImageObject::GetFullSubresourceLayers(EViewAsType type, uint32_t mip_level) const
	{
		VkImageSubresourceLayers return_val{};
		return_val.mipLevel = (std::min)(mip_level, m_MipLevelNum - 1);
		return_val.baseArrayLayer = 0;
		return_val.layerCount = m_LayerNum;
		if (type == EViewAsType::E_VIEW_AS_TYPE_MAX)
		{
			return_val.aspectMask = TranslateViewAsTypeToVulkanAspectFlags(m_DefaultViewAsType);
		}
		else
		{
			return_val.aspectMask = TranslateViewAsTypeToVulkanAspectFlags(type);
		}
		return return_val;
	}
	VkImageSubresourceLayers VulkanImageObject::GetSubresourceLayers(EViewAsType type, uint32_t mip_level, uint32_t base_layer, uint32_t layer_num) const
	{
		VkImageSubresourceLayers return_val{};
		return_val.mipLevel = (std::min)(mip_level, m_MipLevelNum - 1);
		return_val.baseArrayLayer = base_layer;
		return_val.layerCount = layer_num;
		if (type == EViewAsType::E_VIEW_AS_TYPE_MAX)
		{
			return_val.aspectMask = TranslateViewAsTypeToVulkanAspectFlags(m_DefaultViewAsType);
		}
		else
		{
			return_val.aspectMask = TranslateViewAsTypeToVulkanAspectFlags(type);
		}
		return return_val;
	}
	VkImageView VulkanImageObject::GetView(EViewAsType view_as_type)
	{
		size_t index = static_cast<size_t>(view_as_type);
		if (view_as_type == EViewAsType::E_VIEW_AS_TYPE_MAX)
		{
			index = static_cast<size_t>(m_DefaultViewAsType);
		}
		if (m_ImageViewMap[index] < 0)
		{
			VkImageView new_view = VK_NULL_HANDLE;
			VkImageViewCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			create_info.format = TranslateImageFormatToVulkan(m_Format);
			create_info.image = m_Image;
			create_info.components.r = VK_COMPONENT_SWIZZLE_R;
			create_info.components.g = VK_COMPONENT_SWIZZLE_G;
			create_info.components.b = VK_COMPONENT_SWIZZLE_B;
			create_info.components.a = VK_COMPONENT_SWIZZLE_A;

			create_info.viewType = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
			{
				uint8_t dimention = 0;
				if (m_Width > 1) { dimention++; }
				if (m_Height > 1) { dimention++; }
				if (m_Depth > 1) { dimention++; }
				switch (dimention)
				{
				case 1:
					create_info.viewType = (m_LayerNum > 1) ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
					break;
				case 2:
					create_info.viewType = (m_LayerNum > 1) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
					break;
				case 3:
					create_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
					break;
				}
			}
			create_info.subresourceRange = GetFullSubresourceRange(view_as_type);
			//TODO: further explorations
			create_info.flags = 0;
			create_info.pNext = nullptr;
			CHECK_VKRESULT(vkCreateImageView(p_OwningDevice->m_LogicalDevice, &create_info, VULKAN_ALLOCATOR_POINTER, &new_view), "Vulkan Create Image View Issue!");
			m_ImageViews.push_back(new_view);
			m_ImageViewMap[index] = m_ImageViews.size() - 1;
		}
		return m_ImageViews[m_ImageViewMap[index]];
	}
	VkSampler VulkanImageObject::GetSampler(EFilterMode filter_mode, EAddrMode address_mode)
	{
		VkSampler& target_sampler = m_Samplers[static_cast<size_t>(filter_mode)][static_cast<size_t>(address_mode)];
		if (target_sampler == VK_NULL_HANDLE)
		{
			VkSamplerCreateInfo sampler_create_info{};
			sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			sampler_create_info.addressModeU = TranslateSamplerAddressModeToVulkan(address_mode);
			sampler_create_info.addressModeV = TranslateSamplerAddressModeToVulkan(address_mode);
			sampler_create_info.addressModeW = TranslateSamplerAddressModeToVulkan(address_mode);
			sampler_create_info.anisotropyEnable = true;
			sampler_create_info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
			sampler_create_info.unnormalizedCoordinates = VK_FALSE;
			sampler_create_info.minFilter = sampler_create_info.magFilter = TranslateFilterModeToVulkanFilter(filter_mode);
			
			//compare operation
			sampler_create_info.compareEnable = VK_FALSE;
			sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
			//mipmap
			sampler_create_info.mipmapMode = TranslateFilterModeToVulkanMipMapMode(filter_mode);
			sampler_create_info.mipLodBias = 0.0f;
			sampler_create_info.minLod = 0.0f;
			sampler_create_info.maxLod = 50.0f;
			//anisotropy
			sampler_create_info.anisotropyEnable = VK_TRUE;
			sampler_create_info.maxAnisotropy = 1;

			CHECK_VKRESULT(vkCreateSampler(p_OwningDevice->m_LogicalDevice, &sampler_create_info, VULKAN_ALLOCATOR_POINTER, &target_sampler), "Creating Vulkan Sampler Issue!");
		}

		return target_sampler;
	}
	void VulkanImageObject::CmdChangeOverallLayout(VkCommandBuffer cmd_buffer, uint32_t queue_family, VkImageLayout dst_layout,
		VkPipelineStageFlags dst_stage, VkPipelineStageFlags finished_stage)
	{
		VkImageMemoryBarrier image_barrier{};
		image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		image_barrier.image = m_Image;
		image_barrier.oldLayout = m_OverallImageLayout;
		image_barrier.newLayout = dst_layout;
		image_barrier.srcAccessMask = m_CurrentAccessMask;
		image_barrier.dstAccessMask = VkTranslateLayoutToAccessFlags(dst_layout);
		image_barrier.subresourceRange = GetFullSubresourceRange();
		if (m_SharingMode == VK_SHARING_MODE_CONCURRENT)
		{
			image_barrier.srcQueueFamilyIndex = image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			m_CurrentQueueFamily = m_NextQueueFamily = VK_QUEUE_FAMILY_IGNORED;
		}
		else
		{
			CRIM_ASSERT((m_NextQueueFamily == VK_QUEUE_FAMILY_IGNORED || m_NextQueueFamily == queue_family), "Vulkan Image Not Prepared For Layout Transition On This Queue Family");
			image_barrier.srcQueueFamilyIndex = m_CurrentQueueFamily;
			if (image_barrier.srcQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED)
			{
				image_barrier.srcQueueFamilyIndex = queue_family;
			}
			image_barrier.dstQueueFamilyIndex = queue_family;
			m_CurrentQueueFamily = m_NextQueueFamily = queue_family;
		}
		image_barrier.pNext = nullptr;
		vkCmdPipelineBarrier(cmd_buffer, m_LastUsingStage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &image_barrier);

		m_OverallImageLayout = dst_layout;
		m_CurrentAccessMask = VkTranslateLayoutToAccessFlags(dst_layout);
		m_LastUsingStage = finished_stage;
	}
	VulkanImageLayoutCache VulkanImageObject::GetCurrentLayoutCache() const
	{
		VulkanImageLayoutCache return_val;
		return_val.m_EndLayout = m_OverallImageLayout;
		return_val.m_PielineStages = m_LastUsingStage;
		return return_val;
	}
	void VulkanImageObject::ApplyLayoutCache(uint32_t queue_family, VulkanImageLayoutCache const& cache)
	{
		m_OverallImageLayout = cache.m_EndLayout;
		m_CurrentAccessMask = VkTranslateLayoutToAccessFlags(m_OverallImageLayout);
		m_LastUsingStage = cache.m_PielineStages;
	}
	void VulkanImageLayoutCache::UpdateLayout(VkImageLayout new_layout)
	{
		CRIM_ASSERT(new_layout != VK_IMAGE_LAYOUT_UNDEFINED, "Invalid Image Layout");
		m_EndLayout = new_layout;
	}
}