#include <headers/VulkanFramebuffer.h>
#include <headers/VulkanImage.h>
#include <headers/GeneralDebug.h>

namespace Crimson
{
	void VulkanFramebuffer::Dispose()
	{
		p_OwningDevice->HandleDisposedFramebuffer(this);
	}
	VulkanFramebuffer::VulkanFramebuffer(VulkanGPUDevice* p_device) : 
		p_OwningDevice(nullptr),
		m_RenderArea(),
		m_RenderAreaInited(false)
	{
		p_OwningDevice = p_device;
	}
	VulkanFramebuffer::~VulkanFramebuffer()
	{
	}
	uint32_t VulkanFramebuffer::GetWidth()
	{
		return m_Width > 0 ? m_Width : m_Images[0]->GetImageWidth();
	}
	uint32_t VulkanFramebuffer::GetHeight()
	{
		return m_Height > 0 ? m_Height : m_Images[0]->GetImageHeight();
	}
	uint32_t VulkanFramebuffer::GetLayers()
	{
		return m_Layers > 0 ? m_Layers : m_Images[0]->GetImageLayerNum();
	}
	VkRect2D& VulkanFramebuffer::GetRenderArea()
	{
		if (!m_RenderAreaInited)
		{
			m_RenderArea.offset.x = m_RenderArea.offset.y = 0;
			m_RenderArea.extent.width = GetWidth();
			m_RenderArea.extent.height = GetHeight();
			m_RenderAreaInited = true;
		}
		return m_RenderArea;
	}
	void VulkanFramebuffer::ImageBarriers(VkCommandBuffer cmd_buffer, uint32_t cmd_queue_family)
	{
		m_ImageMemoryBarriers.clear();
		for (auto image : m_Images)
		{
			VulkanImageObject* vulkan_image = static_cast<VulkanImageObject*>(image);
			//if image is not a attachment layout, do a barrier
			if (
				//need layout barrier
				(vulkan_image->m_OverallImageLayout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				&& vulkan_image->m_OverallImageLayout != VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
				//need ownership transfer
				|| (vulkan_image->m_SharingMode == VK_SHARING_MODE_EXCLUSIVE && vulkan_image->m_CurrentQueueFamily != cmd_queue_family))
			{
				VkImageMemoryBarrier image_barrier{};
				image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				image_barrier.image = vulkan_image->m_Image;
				//transfer queue family
				if (vulkan_image->m_SharingMode == VK_SHARING_MODE_CONCURRENT)
				{
					image_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				}
				else
				{
					CRIM_ASSERT(vulkan_image->m_NextQueueFamily == NUMMAX_UINT32 || vulkan_image->m_NextQueueFamily == cmd_queue_family, "Vulkan Image Is Not Prepared For Barrier In This Queue!");
					image_barrier.srcQueueFamilyIndex = vulkan_image->m_CurrentQueueFamily;
					if (image_barrier.srcQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED)
					{
						image_barrier.srcQueueFamilyIndex = cmd_queue_family;
					}
					image_barrier.dstQueueFamilyIndex = cmd_queue_family;
				}
				vulkan_image->m_NextQueueFamily = vulkan_image->m_CurrentQueueFamily = cmd_queue_family;

				//access mask
				image_barrier.srcAccessMask = vulkan_image->m_CurrentAccessMask;
				image_barrier.oldLayout = vulkan_image->m_OverallImageLayout;
				if (IsColorFormat(vulkan_image->m_Format))
				{
					image_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					image_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				}
				else
				{
					image_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					image_barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				}
				vulkan_image->m_CurrentAccessMask = image_barrier.dstAccessMask;
				vulkan_image->m_OverallImageLayout = image_barrier.newLayout;
				image_barrier.subresourceRange = vulkan_image->GetFullSubresourceRange();
				m_ImageMemoryBarriers.push_back(image_barrier);
				vulkan_image->m_LastUsingStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
		}
		if (m_ImageMemoryBarriers.size() > 0) {
			vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, m_ImageMemoryBarriers.size(),
				m_ImageMemoryBarriers.data());
		}
	}
}