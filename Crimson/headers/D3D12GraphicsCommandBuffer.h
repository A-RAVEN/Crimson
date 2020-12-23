#pragma once
#include <include/GPUDevice.h>
#include <include/Generals.h>
#include <headers/D3D12Header.h>
#include <headers/D3D12RenderPassInstance.h>
#include <headers/D3D12GPUDeviceThread.h>

namespace Crimson
{
	//class D3D12GPUDeviceThread;
	class D3D12GraphicsCommandBuffer : public GraphicsCommandBuffer
	{
	public:
		virtual void EndCommandBuffer() override;
		virtual void BindSubpassDescriptorSets(std::vector<PDescriptorSet> const& descriptor_sets, uint32_t start_set) override;
		virtual void PushConstants(std::vector<EShaderType> const& shader_stages, uint32_t offset, uint32_t size, void const* p_data) override {}
		//virtual void ViewPort(float x, float y, float width, float height) override;
		//virtual void Sissor(int _offsetx, int _offsety, uint32_t _extend_x, uint32_t _extend_y) override;
		virtual void BindSubpassPipeline(PGraphicsPipeline pipeline) override;
		virtual void BindVertexInputeBuffer(std::vector<PGPUBuffer> const& buffer_list, std::vector<BufferRange> const& buffer_range_list, std::vector<uint64_t> const& vertex_strides) override;
		virtual void BindIndexBuffer(PGPUBuffer buffer, BufferRange const& buffer_range, EIndexType index_type = EIndexType::E_INDEX_TYPE_32) override;
		virtual void DrawIndexed(uint32_t index_count, uint32_t instance_count,
			uint32_t first_index, uint32_t first_vertex, uint32_t first_instance_id) override;
		virtual void Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance_id) override;
		virtual void DrawMeshShading(uint32_t task_count, uint32_t first_task_id) override {}

		void InitCommandBuffer(D3D12GPUDeviceThread* owningThread, ComPtr<ID3D12GraphicsCommandList4> cmdList, CommandAllocatorEntry const& entry, D3D12RenderPassInstance* renderpass_instance, uint32_t subpass);
	private:
		D3D12GPUDeviceThread* p_OwningThread;
		ComPtr<ID3D12GraphicsCommandList4> m_CommandList;
		CommandAllocatorEntry m_AllocatorEntry;
		D3D12RenderPassInstance* p_OwningInstance;
		uint32_t m_SubpassId;
	};
}