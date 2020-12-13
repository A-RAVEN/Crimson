#include <headers/VulkanGraphicsCommandBuffer.h>
#include <headers/VulkanPipeline.h>
#include <headers/GeneralDebug.h>
#include <headers/VulkanBuffer.h>
#include <headers/VulkanRenderPassInstance.h>
#include <headers/VulkanDebugLog.h>
#include <headers/VulkanTranslator.h>

namespace Crimson
{
	VulkanGraphicsCommandBuffer::VulkanGraphicsCommandBuffer() :
		p_OwningThread(nullptr),
		p_OwningRenderPass(nullptr),
		p_OwningInstance(nullptr),
		m_SubpassId(0),
		m_CommandBuffer(VK_NULL_HANDLE),
		m_CurrentPipelineLayout(VK_NULL_HANDLE)
	{}
	VulkanGraphicsCommandBuffer::~VulkanGraphicsCommandBuffer()
	{
	}
	void VulkanGraphicsCommandBuffer::EndCommandBuffer()
	{
		CHECK_VKRESULT(vkEndCommandBuffer(m_CommandBuffer), "Vulkan End Graphics Command Buffer Issue!");
		p_OwningThread->HandleDisposedGraphicsCommandBuffer(this);
	}
	void VulkanGraphicsCommandBuffer::StartCommandBuffer()
	{
		p_ReferencingDescriptorSets.clear();
		VkCommandBufferInheritanceInfo inheritance{};
		inheritance.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritance.framebuffer = p_OwningInstance->m_Framebuffer;
		inheritance.renderPass = p_OwningRenderPass->m_RenderPass;
		inheritance.subpass = m_SubpassId;
		//TODO Further Explorations
		inheritance.occlusionQueryEnable = VK_FALSE;
		inheritance.pipelineStatistics = 0;
		inheritance.queryFlags = 0;
		inheritance.pNext = nullptr;
		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		begin_info.pInheritanceInfo = &inheritance;
		begin_info.pNext = nullptr;
		CHECK_VKRESULT(vkBeginCommandBuffer(m_CommandBuffer, &begin_info), "Vulkan Start Graphics Command Buffer Issue!");
	}
	void VulkanGraphicsCommandBuffer::BindSubpassDescriptorSets(std::vector<PDescriptorSet> const& descriptor_sets, uint32_t start_set)
	{
		CRIM_ASSERT(m_CurrentPipelineLayout != VK_NULL_HANDLE, "Vulkan Graphics Command Buffer Binding Descriptor Set Issue, Invalid Pipeline Layout!");
		std::vector<VkDescriptorSet> sets(descriptor_sets.size());
		for (uint32_t set_id = 0; set_id < descriptor_sets.size(); ++set_id)
		{
			VulkanDescriptorSet* p_vulkan_desc_set = static_cast<VulkanDescriptorSet*>(descriptor_sets[set_id]);
			sets[set_id] = p_vulkan_desc_set->m_DescriptorSet;
			p_ReferencingDescriptorSets.push_back(p_vulkan_desc_set);
		}
		vkCmdBindDescriptorSets(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_CurrentPipelineLayout, start_set, descriptor_sets.size(), sets.data(), 0, nullptr);
	}
	void VulkanGraphicsCommandBuffer::PushConstants(std::vector<EShaderType> const& shader_stages, uint32_t offset, uint32_t size, void const* p_data)
	{
		CRIM_ASSERT(m_CurrentPipelineLayout != VK_NULL_HANDLE, "Vulkan Graphics Command Buffer Pushing Constants Issue, Invalid Pipeline Layout!");
		VkShaderStageFlags stage_flags = 0;
		for (auto shader_type : shader_stages)
		{
			stage_flags |= TranslateShaderTypeToVulkan(shader_type);
		}
		vkCmdPushConstants(m_CommandBuffer, m_CurrentPipelineLayout, stage_flags, offset, size, p_data);
	}
	void VulkanGraphicsCommandBuffer::ViewPort(float x, float y, float width, float height)
	{
		VkViewport view_port{};
		view_port.x = x;
		view_port.y = y;
		view_port.width = width;
		view_port.height = height;
		view_port.minDepth = 0.0f;
		view_port.maxDepth = 1.0f;
		vkCmdSetViewport(m_CommandBuffer, 0, 1, &view_port);
	}
	void VulkanGraphicsCommandBuffer::Sissor(int _offsetx, int _offsety, uint32_t _extend_x, uint32_t _extend_y)
	{
		VkRect2D sissor = { {_offsetx, _offsety},{_extend_x, _extend_y} };
		vkCmdSetScissor(m_CommandBuffer, 0, 1, &sissor);
	}
	void VulkanGraphicsCommandBuffer::BindSubpassPipeline(PGraphicsPipeline pipeline)
	{
		VulkanGraphicsPipeline* p_pipeline = static_cast<VulkanGraphicsPipeline*>(pipeline);
		auto pipeline_instance_find = p_OwningRenderPass->m_VulkanSubpassInfos[m_SubpassId].m_PipelineInstanceRefs.find(pipeline);
		if (pipeline_instance_find != p_OwningRenderPass->m_VulkanSubpassInfos[m_SubpassId].m_PipelineInstanceRefs.end())
		{
			auto& inst_pair = p_OwningRenderPass->m_VulkanSubpassInfos[m_SubpassId].m_PipelineInstances[pipeline_instance_find->second];
			vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, inst_pair.first);
			m_CurrentPipelineLayout = inst_pair.second;
		}
	}
	void VulkanGraphicsCommandBuffer::BindVertexInputeBuffer(std::vector<PGPUBuffer> const& buffer_list, std::vector<BufferRange> const& buffer_range_list, std::vector<uint64_t> const& vertex_strides)
	{
		std::vector<VkBuffer> buffers(buffer_list.size());
		std::vector<VkDeviceSize> bufferOffsets(buffer_list.size());
		for (uint32_t buffer_id = 0; buffer_id < buffer_list.size(); ++buffer_id)
		{
			buffers[buffer_id] = static_cast<VulkanBufferObject*> (buffer_list[buffer_id])->m_Buffer;
			bufferOffsets[buffer_id] = buffer_range_list[buffer_id].m_Offset;
		}
		vkCmdBindVertexBuffers(m_CommandBuffer, 0, buffers.size(), buffers.data(), bufferOffsets.data());
	}
	void VulkanGraphicsCommandBuffer::BindIndexBuffer(PGPUBuffer buffer, BufferRange const& buffer_range, EIndexType index_type)
	{
		VkIndexType vulkan_index_type = (index_type == EIndexType::E_INDEX_TYPE_32) ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16;
		vkCmdBindIndexBuffer(m_CommandBuffer, static_cast<VulkanBufferObject*>(buffer)->m_Buffer, buffer_range.m_Offset, vulkan_index_type);
	}
	void VulkanGraphicsCommandBuffer::DrawIndexed(uint32_t index_count, uint32_t instance_count, 
		uint32_t first_index, uint32_t first_vertex, uint32_t first_instance_id)
	{
		vkCmdDrawIndexed(m_CommandBuffer, index_count, instance_count, first_index, first_vertex, first_instance_id);
	}
	void VulkanGraphicsCommandBuffer::Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance_id)
	{
		vkCmdDraw(m_CommandBuffer, vertex_count, instance_count, first_vertex, first_instance_id);
	}
	void VulkanGraphicsCommandBuffer::DrawMeshShading(uint32_t task_count, uint32_t first_task_id)
	{
		p_OwningThread->p_OwningDevice->m_NVExtension.vkCmdDrawMeshTasksNV(m_CommandBuffer, task_count, first_task_id);
	}
	void VulkanGraphicsCommandBuffer::SetGraphicsCommandBuffer(VulkanGPUDeviceThread* p_owning_thread, 
		VulkanRenderPass* p_owning_render_pass, VulkanRenderPassInstance* p_owning_instance, uint32_t subpass, VkCommandBuffer cmd_buffer)
	{
		p_OwningThread = p_owning_thread;
		p_OwningRenderPass = p_owning_render_pass;
		p_OwningInstance = p_owning_instance;
		m_SubpassId = subpass;
		m_CommandBuffer = cmd_buffer;
	}
	void VulkanGraphicsCommandBuffer::BarrierDescriptorSets_Unsafe(VkCommandBuffer cmd_buffer)
	{
	}
}