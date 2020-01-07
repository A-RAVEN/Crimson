#include <headers/VulkanGraphicsCommandBuffer.h>
#include <headers/VulkanPipeline.h>
#include <headers/GeneralDebug.h>
#include <headers/VulkanBuffer.h>

namespace Crimson
{
	void VulkanGraphicsCommandBuffer::EndCommandBuffer()
	{
		vkEndCommandBuffer(m_CommandBuffer);
	}
	void VulkanGraphicsCommandBuffer::BindSubpassDescriptorSets(std::vector<PDescriptorSet> const& descriptor_sets)
	{
		CRIM_ASSERT(m_CurrentPipelineLayout != VK_NULL_HANDLE, "Vulkan Graphics Command Buffer Binding Descriptor Set Issue, Invalid Pipeline Layout!");
		std::vector<VkDescriptorSet> sets(descriptor_sets.size());
		for (uint32_t set_id = 0; set_id < descriptor_sets.size(); ++set_id)
		{
			sets[set_id] = static_cast<VulkanDescriptorSet*>(descriptor_sets[set_id])->m_DescriptorSet;
		}
		vkCmdBindDescriptorSets(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_CurrentPipelineLayout, 0, descriptor_sets.size(), sets.data(), 0, nullptr);
	}
	void VulkanGraphicsCommandBuffer::BindSubpassPipeline(PGraphicsPipeline pipeline)
	{
		VulkanGraphicsPipeline* p_pipeline = static_cast<VulkanGraphicsPipeline*>(pipeline);
		auto pipeline_instance_find = p_OwningRenderPass->m_VulkanSubpassInfos[m_SubpassId].m_PipelineInstances.find(pipeline);
		if (pipeline_instance_find != p_OwningRenderPass->m_VulkanSubpassInfos[m_SubpassId].m_PipelineInstances.end())
		{
			vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_instance_find->second.first);
			m_CurrentPipelineLayout = pipeline_instance_find->second.second;
		}
	}
	void VulkanGraphicsCommandBuffer::BindVertexInputeBuffer(std::vector<PGPUBuffer> const& buffer_list, std::vector<uint64_t> const& buffer_offset_list)
	{
		std::vector<VkBuffer> buffers(buffer_list.size());
		for (uint32_t buffer_id = 0; buffer_id < buffer_list.size(); ++buffer_id)
		{
			buffers[buffer_id] = static_cast<VulkanBufferObject*> (buffer_list[buffer_id])->m_Buffer;
		}
		vkCmdBindVertexBuffers(m_CommandBuffer, 0, buffers.size(), buffers.data(), buffer_offset_list.data());
	}
	void VulkanGraphicsCommandBuffer::BindIndexBuffer(PGPUBuffer buffer, uint64_t buffer_offset, EIndexType index_type)
	{
		VkIndexType vulkan_index_type = (index_type == EIndexType::E_INDEX_TYPE_32) ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16;
		vkCmdBindIndexBuffer(m_CommandBuffer, static_cast<VulkanBufferObject*>(buffer)->m_Buffer, buffer_offset, vulkan_index_type);
	}
	void VulkanGraphicsCommandBuffer::DrawIndexed(uint32_t index_count, uint32_t instance_count, 
		uint32_t first_index, uint32_t first_vertex, uint32_t first_instance_id)
	{
		vkCmdDrawIndexed(m_CommandBuffer, index_count, instance_count, first_index, first_vertex, first_instance_id);
	}
}