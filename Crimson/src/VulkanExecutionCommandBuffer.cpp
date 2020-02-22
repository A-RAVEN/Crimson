#include <headers/VulkanExecutionCommandBuffer.h>
#include <headers/VulkanRenderPassInstance.h>
#include <headers/VulkanRenderPass.h>
#include <headers/VulkanFramebuffer.h>
#include <headers/VulkanDebugLog.h>
#include <headers/VulkanBuffer.h>
#include <headers/VulkanImage.h>
#include <headers/VulkanRayTracer.h>
#include <headers/VulkanDescriptors.h>

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
		vulkan_renderpass_instance->p_OwningExecutionCommandBuffer = this;

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

		std::vector<std::vector<VkCommandBuffer>> subpass_cmd_buffers(vulkan_renderpass->m_Subpasses.size());
		std::deque<VulkanDescriptorSet*> referenced_sets;
		for (uint32_t subpass_id = 0; subpass_id < vulkan_renderpass->m_Subpasses.size(); ++subpass_id)
		{
			referenced_sets.clear();
			subpass_cmd_buffers[subpass_id] = p_OwningDevice->CollectSubpassCommandBuffers(subpass_id, vulkan_renderpass_instance, referenced_sets);
			for (auto set : referenced_sets)
			{
				//NOTICE: Currently, Render Pass Is Only Used For Graphics Use
				set->CmdBarrierDescriptorSet(m_CurrentCommandBuffer, p_OwningDevice->GetQueueFamilyIdByCommandType(m_CommandType), 0);
			}
		}
		{
			//std::vector<VkCommandBuffer> cmd_buffers = p_OwningDevice->CollectSubpassCommandBuffers(0, vulkan_renderpass_instance, referenced_sets);
			//for (auto set : referenced_sets)
			//{
			//	//NOTICE: Currently, Render Pass Is Only Used For Graphics Use
			//	set->CmdBarrierDescriptorSet(m_CurrentCommandBuffer, p_OwningDevice->GetQueueFamilyIdByCommandType(m_CommandType), 0);
			//}

			vkCmdBeginRenderPass(m_CurrentCommandBuffer, &begin_info, VkSubpassContents::VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
			vkCmdExecuteCommands(m_CurrentCommandBuffer, subpass_cmd_buffers[0].size(), subpass_cmd_buffers[0].data());
		}
		for (uint32_t subpass_id = 1; subpass_id < vulkan_renderpass->m_Subpasses.size(); ++subpass_id)
		{
			referenced_sets.clear();
			vkCmdNextSubpass(m_CurrentCommandBuffer, VkSubpassContents::VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
			//std::vector<VkCommandBuffer> cmd_buffers = p_OwningDevice->CollectSubpassCommandBuffers(subpass_id, vulkan_renderpass_instance, referenced_sets);
			vkCmdExecuteCommands(m_CurrentCommandBuffer, subpass_cmd_buffers[subpass_id].size(), subpass_cmd_buffers[subpass_id].data());
		}
		vkCmdEndRenderPass(m_CurrentCommandBuffer);
	}
	void VulkanExecutionCommandBuffer::CopyBufferToImage(PGPUBuffer buffer, PGPUImage image, 
		uint64_t buffer_offset, uint32_t mip_level, uint32_t base_layer, uint32_t layer_count)
	{
		VulkanBufferObject* vulkan_buffer = static_cast<VulkanBufferObject*>(buffer);
		VulkanImageObject* vulkan_image = static_cast<VulkanImageObject*>(image);
		vulkan_image->CmdChangeOverallLayout(m_CurrentCommandBuffer, p_OwningDevice->GetQueueFamilyIdByCommandType(m_CommandType), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		
		VkBufferImageCopy region = {};
		region.bufferOffset = buffer_offset;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { vulkan_image->m_Width,  vulkan_image->m_Height, vulkan_image->m_Depth };
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource = vulkan_image->GetSubresourceLayers(EViewAsType::E_VIEW_AS_TYPE_MAX, mip_level, base_layer, layer_count);

		vkCmdCopyBufferToImage(m_CurrentCommandBuffer, vulkan_buffer->m_Buffer, vulkan_image->m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}
	void VulkanExecutionCommandBuffer::CopyToSwapchain_Dynamic(PGPUImage image, IWindow* p_window)
	{
		auto find = p_OwningDevice->m_SurfaceContexts.find(p_window->GetName());
		if (find != p_OwningDevice->m_SurfaceContexts.end())
		{
			auto& context = find->second;
			uint32_t img_id = context.AquireNextImage();
			VkImage swapchain_img = context.m_SwapchainImages[img_id];
			m_AdditionialWaitingSemaphores.push_back(context.m_AquireFinishSemaphore);
			m_AdditionalWaitingStages.push_back(VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT);
			VulkanImageObject* vulkan_image = static_cast<VulkanImageObject*>(image);
			vulkan_image->CmdChangeOverallLayout(m_CurrentCommandBuffer, p_OwningDevice->GetQueueFamilyIdByCommandType(m_CommandType), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
			TransitionSwapchainImageToCopyDst(swapchain_img, &context, context.m_SwapchainImageInitialized[img_id]);
			context.m_SwapchainImageInitialized[img_id] = true;
			VkImageCopy image_copy{};
			image_copy.srcOffset = { 0, 0, 0 };
			image_copy.dstOffset = { 0, 0, 0 };
			image_copy.srcSubresource = vulkan_image->GetFullSubresourceLayers();
			image_copy.dstSubresource = context.GetSwapchainImageSubresourceLayers();
			image_copy.extent = { (std::min)(vulkan_image->m_Width, context.m_Extent.width), (std::min)(vulkan_image->m_Height, context.m_Extent.height), 1 };
			vkCmdCopyImage(m_CurrentCommandBuffer, vulkan_image->m_Image, vulkan_image->m_OverallImageLayout, swapchain_img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_copy);
			TransitionSwapchainImageToPresent(swapchain_img, &context);
		}
	}
	void VulkanExecutionCommandBuffer::BuildAccelerationStructure(PAccelerationStructure accel_struct, 
		PGPUBuffer instance_buffer, uint64_t instance_offset, bool update)
	{
		VulkanAccelerationStructure* vulkan_accel_struct = static_cast<VulkanAccelerationStructure*>(accel_struct);
		VkBuffer vulkan_instance_buffer = VK_NULL_HANDLE;
		if (instance_buffer != nullptr)
		{
			vulkan_instance_buffer = static_cast<VulkanBufferObject*>(instance_buffer)->m_Buffer;
		}
		p_OwningDevice->m_NVExtension.vkCmdBuildAccelerationStructureNV(m_CurrentCommandBuffer, &vulkan_accel_struct->m_StructureInfo, vulkan_instance_buffer, instance_offset, update ? VK_TRUE : VK_FALSE, vulkan_accel_struct->m_Structure, VK_NULL_HANDLE, vulkan_accel_struct->p_ScratchBuffer->GetVulkanBuffer(), 0);
	}
	void VulkanExecutionCommandBuffer::BindRayTracer(PRayTracer raytracer)
	{
		VulkanRayTracer* vulkan_raytracer = static_cast<VulkanRayTracer*>(raytracer);
		vkCmdBindPipeline(m_CurrentCommandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, vulkan_raytracer->m_Pipeline);
		p_CurrentBoundedRayTracer = vulkan_raytracer;
	}
	void VulkanExecutionCommandBuffer::BindRayTracingDescriptorSet(PDescriptorSet descriptor_set, uint32_t set_id)
	{
		VulkanDescriptorSet* vulkan_set = static_cast<VulkanDescriptorSet*>(descriptor_set);
		vulkan_set->CmdBarrierDescriptorSet(m_CurrentCommandBuffer, p_OwningDevice->GetQueueFamilyIdByCommandType(m_CommandType), 2);
		vkCmdBindDescriptorSets(m_CurrentCommandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, p_CurrentBoundedRayTracer->m_PipelineLayout,
			set_id, 1, &vulkan_set->m_DescriptorSet, 0, nullptr);
	}
	void VulkanExecutionCommandBuffer::StartRayTracing(PGPUBuffer raygen_table, uint64_t raygen_offset, uint64_t miss_offset, uint64_t hit_offset, uint32_t width, uint32_t height)
	{
		uint32_t handle_size = p_OwningDevice->m_NVExtension.m_RayTracingProperties.shaderGroupHandleSize;
		VulkanBufferObject* p_vulkan_buffer = static_cast<VulkanBufferObject*>(raygen_table);
		p_OwningDevice->m_NVExtension.vkCmdTraceRaysNV(m_CurrentCommandBuffer,
			p_vulkan_buffer->m_Buffer, raygen_offset * handle_size,
			p_vulkan_buffer->m_Buffer, miss_offset * handle_size, handle_size,
			p_vulkan_buffer->m_Buffer, hit_offset * handle_size, handle_size,
			VK_NULL_HANDLE, 0, handle_size,
			width, height, 1
		);
	}
	void VulkanExecutionCommandBuffer::DeviceMemoryBarrier(EMemoryBarrierType barrier_type)
	{
		VkMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		barrier.pNext = nullptr;
		switch (barrier_type)
		{
		case EMemoryBarrierType::E_ACCEL_STRUCTURE_BUILD_READ_WRITE:
			barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;
			barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;
			vkCmdPipelineBarrier(m_CurrentCommandBuffer, 
				VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV,
				0, 1, &barrier, 0, nullptr, 0, nullptr);
			break;
		default:
			break;
		}
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
		m_AdditionialWaitingSemaphores.clear();
		m_AdditionalWaitingStages.clear();
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
	void VulkanExecutionCommandBuffer::TransitionSwapchainImageToCopyDst(VkImage swapchain_image, VulkanSurfaceContext* surface_context, bool initialized)
	{
		VkImageMemoryBarrier image_barrier{};
		image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		image_barrier.image = swapchain_image;
		image_barrier.oldLayout = initialized ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_UNDEFINED;
		image_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		image_barrier.srcAccessMask = 0;
		image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		image_barrier.subresourceRange = surface_context->GetSwapchainImageSubresourceRange();
		image_barrier.srcQueueFamilyIndex = image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		image_barrier.pNext = nullptr;
		vkCmdPipelineBarrier(m_CurrentCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_barrier);
	}
	void VulkanExecutionCommandBuffer::TransitionSwapchainImageToPresent(VkImage swapchain_image, VulkanSurfaceContext* surface_context)
	{
		VkImageMemoryBarrier image_barrier{};
		image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		image_barrier.image = swapchain_image;
		image_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		image_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		image_barrier.dstAccessMask = 0;
		image_barrier.subresourceRange = surface_context->GetSwapchainImageSubresourceRange();
		image_barrier.srcQueueFamilyIndex = image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		image_barrier.pNext = nullptr;
		vkCmdPipelineBarrier(m_CurrentCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_barrier);
	}
}