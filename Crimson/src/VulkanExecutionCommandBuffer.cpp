#include <headers/VulkanExecutionCommandBuffer.h>
#include <headers/VulkanRenderPassInstance.h>
#include <headers/VulkanRenderPass.h>
#include <headers/VulkanFramebuffer.h>
#include <headers/VulkanDebugLog.h>
#include <headers/VulkanImage.h>

namespace Crimson
{
	VulkanExecutionCommandBuffer::VulkanExecutionCommandBuffer() :
		p_OwningDevice(nullptr),
		p_OwningThread(nullptr),
		p_AttachedBatch(nullptr),
		m_CommandType(EExecutionCommandType::E_COMMAND_TYPE_MAX),
		m_CurrentCommandBuffer(VK_NULL_HANDLE)
	{}
	void VulkanExecutionCommandBuffer::ExecuteRenderPassInstance(PRenderPassInstance renderpass_instance)
	{
		VulkanRenderPassInstance* vulkan_renderpass_instance = static_cast<VulkanRenderPassInstance*>(renderpass_instance);
		VulkanFramebuffer* vulkan_framebuffer = static_cast<VulkanFramebuffer*> (vulkan_renderpass_instance->p_Framebuffer);
		VulkanRenderPass* vulkan_renderpass = static_cast<VulkanRenderPass*> (vulkan_renderpass_instance->p_RenderPass);

		VkRenderPassBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		begin_info.renderPass = vulkan_renderpass->m_RenderPass;
		begin_info.framebuffer = vulkan_renderpass_instance->m_Framebuffer;
		begin_info.clearValueCount = vulkan_renderpass->m_ClearValues.size();
		begin_info.pClearValues = begin_info.clearValueCount > 0 ? vulkan_renderpass->m_ClearValues.data() : nullptr;
		begin_info.renderArea = vulkan_framebuffer->GetRenderArea();
		begin_info.pNext = nullptr;
		//recording commands
		vulkan_framebuffer->ImageBarriers(m_CurrentCommandBuffer, p_OwningDevice->GetQueueFamilyIdByCommandType(m_CommandType));
		vkCmdBeginRenderPass(m_CurrentCommandBuffer, &begin_info, VkSubpassContents::VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
		{
			std::vector<VkCommandBuffer> cmd_buffers = p_OwningDevice->CollectSubpassCommandBuffers(0, vulkan_renderpass_instance);
			vkCmdExecuteCommands(m_CurrentCommandBuffer, cmd_buffers.size(), cmd_buffers.data());
		}
		for (uint32_t subpass_id = 1; subpass_id < vulkan_renderpass->m_Subpasses.size(); ++subpass_id)
		{
			vkCmdNextSubpass(m_CurrentCommandBuffer, VkSubpassContents::VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
			std::vector<VkCommandBuffer> cmd_buffers = p_OwningDevice->CollectSubpassCommandBuffers(subpass_id, vulkan_renderpass_instance);
			vkCmdExecuteCommands(m_CurrentCommandBuffer, cmd_buffers.size(), cmd_buffers.data());
		}
		vkCmdEndRenderPass(m_CurrentCommandBuffer);
	}
	void VulkanExecutionCommandBuffer::CopyToSwapchain(PGPUImage image, IWindow* p_window)
	{
		auto find = p_OwningDevice->m_SurfaceContexts.find(p_window->GetName());
		if (find != p_OwningDevice->m_SurfaceContexts.end())
		{
			VulkanImageObject* vulkan_image = static_cast<VulkanImageObject*>(image);
			vulkan_image->CmdChangeOverallLayout(m_CurrentCommandBuffer, p_OwningDevice->GetQueueFamilyIdByCommandType(m_CommandType), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
			auto& context = find->second;
			VkImage swapchain_img = context.AquireNextImage();
			VkImageCopy image_copy{};
			image_copy.srcOffset = { 0, 0, 0 };
			image_copy.dstOffset = { 0, 0, 0 };
			image_copy.srcSubresource = vulkan_image->GetFullSubresourceLayers();
			vkCmdCopyImage(m_CurrentCommandBuffer, vulkan_image->m_Image, vulkan_image->m_OverallImageLayout, swapchain_img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, )
		}
		//vkCmdCopyImage
	}
	void VulkanExecutionCommandBuffer::StartCommand()
	{
		if (p_AttachedBatch != nullptr)
		{
			p_AttachedBatch->Wait();
		}
		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		begin_info.pInheritanceInfo = nullptr;
		begin_info.pNext = nullptr;
		CHECK_VKRESULT(vkBeginCommandBuffer(m_CurrentCommandBuffer, &begin_info), "Vulkan Begin Primary Command Buffer Issue!");
	}
	void VulkanExecutionCommandBuffer::EndCommand()
	{
		CHECK_VKRESULT(vkEndCommandBuffer(m_CurrentCommandBuffer), "Vulkan End Primary Command Buffer Issue!");
	}
	void VulkanExecutionCommandBuffer::SetExecutionCommandBuffer(VulkanGPUDevice* p_device, VulkanGPUDeviceThread* p_thread, EExecutionCommandType command_type)
	{
		p_OwningDevice = p_device;
		p_OwningThread = p_thread;
		m_CommandType = command_type;
		m_CurrentCommandBuffer = p_OwningThread->AllocExecutionVkCommandBuffer(m_CommandType);
	}
}