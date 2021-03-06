#pragma once
#include <include/GPUDevice.h>
#include <include/Generals.h>
#include <headers/VulkanDescriptors.h>
#include <headers/VulkanGPUDevice.h>
#include <headers/VulkanRenderPass.h>
#include <headers/VulkanGPUDeviceThread.h>

namespace Crimson
{
	class VulkanGraphicsCommandBuffer : public GraphicsCommandBuffer
	{
	public:
		friend class VulkanGPUDeviceThread;
		VulkanGraphicsCommandBuffer();
		~VulkanGraphicsCommandBuffer();
		virtual void EndCommandBuffer() override;
		void StartCommandBuffer();
		virtual void BindSubpassDescriptorSets(std::vector<PDescriptorSet> const& descriptor_sets, uint32_t start_set) override;
		virtual void PushConstants(std::vector<EShaderType> const& shader_stages, uint32_t offset, uint32_t size, void const* p_data) override;
		virtual void ViewPort(float x, float y, float width, float height) override;
		virtual void Sissor(int _offsetx, int _offsety, uint32_t _extend_x, uint32_t _extend_y) override;
		virtual void BindSubpassPipeline(PGraphicsPipeline pipeline) override;
		virtual void BindVertexInputeBuffer(std::vector<PGPUBuffer> const& buffer_list, 
			std::vector<uint64_t> const& buffer_offset_list) override;
		virtual void BindIndexBuffer(PGPUBuffer buffer, uint64_t buffer_offset, EIndexType index_type) override;
		virtual void DrawIndexed(uint32_t index_count, uint32_t instance_count,
			uint32_t first_index, uint32_t first_vertex, uint32_t first_instance_id) override;
		virtual void Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance_id) override;
		virtual void DrawMeshShading(uint32_t task_count, uint32_t first_task_id) override;
		void SetGraphicsCommandBuffer(VulkanGPUDeviceThread* p_owning_thread, 
			VulkanRenderPass* p_owning_render_pass, VulkanRenderPassInstance* p_owning_instance, uint32_t subpass, VkCommandBuffer cmd_buffer);

		void BarrierDescriptorSets_Unsafe(VkCommandBuffer cmd_buffer);
	private:
		VulkanRenderPassInstance* p_OwningInstance;
		VulkanGPUDeviceThread* p_OwningThread;
		VulkanRenderPass* p_OwningRenderPass;
		uint32_t m_SubpassId;

		VkCommandBuffer m_CommandBuffer;
		VkPipelineLayout m_CurrentPipelineLayout;

		std::vector<VulkanDescriptorSet*> p_ReferencingDescriptorSets;
	};
}