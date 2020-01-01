#include <headers/VulkanRenderPassInstance.h>
#include <headers/VulkanImage.h>
#include <headers/VulkanDebugLog.h>

namespace Crimson
{
	void VulkanRenderPassInstance::InitRenderPassInstance(VulkanGPUDevice* device, VulkanRenderPass* render_pass, VulkanFramebuffer* framebuffer)
	{
		render_pass->m_VulkanAttachmentInfos;
		VkFramebufferCreateInfo framebuffer_create_info{};
		framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		//framebuffer_create_info.layers = framebuffer->m_Images[0]->GetImageLayerNum();

		std::vector<VkImageView> attachments(render_pass->m_VulkanAttachmentInfos.size());
		for (size_t attachinfo_id = 0; attachinfo_id < render_pass->m_VulkanAttachmentInfos.size(); ++attachinfo_id)
		{
			auto& attach_info = render_pass->m_VulkanAttachmentInfos[attachinfo_id];
			VulkanImageObject* p_image = static_cast<VulkanImageObject*>(framebuffer->m_Images[attach_info.m_ImageReference]);
			attachments[attachinfo_id] = p_image->GetView(EViewAsType::E_VIEW_AS_TYPE_MAX);
		}
		framebuffer_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebuffer_create_info.pAttachments = framebuffer_create_info.attachmentCount > 0 ? attachments.data() : nullptr;
		framebuffer_create_info.width = framebuffer->GetWidth();
		framebuffer_create_info.height = framebuffer->GetHeight();
		framebuffer_create_info.layers = framebuffer->GetLayers();
		framebuffer_create_info.renderPass = render_pass->m_RenderPass;

		//TODO: further explorations
		framebuffer_create_info.flags = 0;
		framebuffer_create_info.pNext = nullptr;
		VulkanDebug::CheckVKResult(vkCreateFramebuffer(device->m_LogicalDevice, &framebuffer_create_info, VULKAN_ALLOCATOR_POINTER, &m_Framebuffer), "Vulkan Instantiate Framebuffer Issue!");
	}
}