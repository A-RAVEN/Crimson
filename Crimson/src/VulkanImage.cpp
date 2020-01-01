#include <headers/VulkanImage.h>
#include <headers/VulkanTranslator.h>
#include <headers/VulkanDebugLog.h>

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
		m_DefaultViewAsType(EViewAsType::E_VIEW_AS_TYPE_MAX)
	{
		std::fill(m_ImageViewMap.begin(), m_ImageViewMap.end(), int8_t(-1));
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
	VkImageSubresourceRange VulkanImageObject::GetFullSubresourceRange(EViewAsType type)
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
				if (m_Width > 0) { dimention++; }
				if (m_Height > 0) { dimention++; }
				if (m_Depth > 0) { dimention++; }
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
			VulkanDebug::CheckVKResult(vkCreateImageView(p_OwningDevice->m_LogicalDevice, &create_info, VULKAN_ALLOCATOR_POINTER, &new_view), "Vulkan Create Image View Issue!");
			m_ImageViews.push_back(new_view);
			m_ImageViewMap[index] = m_ImageViews.size() - 1;
		}
		return m_ImageViews[m_ImageViewMap[index]];
	}
}