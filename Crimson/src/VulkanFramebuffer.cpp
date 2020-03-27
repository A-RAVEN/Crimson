#include <headers/VulkanFramebuffer.h>
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
	void VulkanFramebuffer::CmdImageBarriers(VkCommandBuffer cmd_buffer, uint32_t cmd_queue_family, std::map<VulkanImageObject*, VulkanImageLayoutCache>& image_layout_cache)
	{
		m_ImageMemoryBarriers.clear();
		for (auto image : m_Images)
		{
			VulkanImageObject* vulkan_image = static_cast<VulkanImageObject*>(image);
			if (IsColorFormat(vulkan_image->m_Format))
			{
				vulkan_image->CmdChangeOverallLayout(cmd_buffer, cmd_queue_family, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
			}
			else
			{
				vulkan_image->CmdChangeOverallLayout(cmd_buffer, cmd_queue_family, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
			}
			auto find = image_layout_cache.find(vulkan_image);
			if (find == image_layout_cache.end())
			{
				image_layout_cache.insert(std::make_pair(vulkan_image, vulkan_image->GetCurrentLayoutCache()));
			}
			else
			{
				find->second = vulkan_image->GetCurrentLayoutCache();
			}
		}
	}
}