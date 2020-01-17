#include <headers/VulkanExecutionCommandBuffer.h>
#include <headers/VulkanRenderPassInstance.h>
#include <headers/VulkanRenderPass.h>
#include <headers/VulkanFramebuffer.h>

namespace Crimson
{
	void VulkanExecutionCommandBuffer::ExecuteRenderPassInstance(PRenderPassInstance renderpass_instance)
	{
		VulkanRenderPassInstance* vulkan_renderpass_instance = static_cast<VulkanRenderPassInstance*>(renderpass_instance);
		VulkanFramebuffer* vulkan_framebuffer = static_cast<VulkanFramebuffer*> (vulkan_renderpass_instance->p_Framebuffer);
		VulkanRenderPass* vulkan_renderpass = static_cast<VulkanRenderPass*> (vulkan_renderpass_instance->p_RenderPass);
		vulkan_framebuffer->ImageBarriers(m_CurrentCommandBuffer, m_QueueFamilyId);
		VkRenderPassBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		begin_info.renderPass = vulkan_renderpass->m_RenderPass;
		begin_info.framebuffer = vulkan_renderpass_instance->m_Framebuffer;
		begin_info.clearValueCount = vulkan_renderpass->m_ClearValues.size();
		begin_info.pClearValues = begin_info.clearValueCount > 0 ? vulkan_renderpass->m_ClearValues.data() : nullptr;
		begin_info.renderArea = vulkan_framebuffer->GetRenderArea();
		begin_info.pNext = nullptr;
		vkCmdBeginRenderPass(m_CurrentCommandBuffer, &begin_info, VkSubpassContents::VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

	}
}