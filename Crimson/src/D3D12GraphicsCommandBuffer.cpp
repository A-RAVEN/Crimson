#include <headers/D3D12GraphicsCommandBuffer.h>
#include <headers/D3D12GPUDevice.h>
#include <headers/D3D12GPUDeviceThread.h>
#include <headers/GeneralDebug.h>
#include <headers/D3D12Buffer.h>

namespace Crimson
{
	void D3D12GraphicsCommandBuffer::EndCommandBuffer()
	{
		m_CommandList->EndRenderPass();
	}
	void D3D12GraphicsCommandBuffer::BindSubpassDescriptorSets(std::vector<PDescriptorSet> const& descriptor_sets, uint32_t start_set)
	{

	}
	void D3D12GraphicsCommandBuffer::ViewPort(float x, float y, float width, float height)
	{
		D3D12_VIEWPORT view_port{};
		view_port.TopLeftX = x;
		view_port.TopLeftY = y;
		view_port.Width = width;
		view_port.Height = height;
		view_port.MinDepth = 0.0f;
		view_port.MaxDepth = 1.0f;
		m_CommandList->RSSetViewports(1, &view_port);
	}
	void D3D12GraphicsCommandBuffer::Sissor(int _offsetx, int _offsety, uint32_t _extend_x, uint32_t _extend_y)
	{
		D3D12_RECT rect{};
		rect.left = _offsetx;
		rect.top = _offsety;
		rect.right = rect.left + _extend_x;
		rect.bottom = rect.top + _extend_y;
		m_CommandList->RSSetScissorRects(1, &rect);
	}
	void D3D12GraphicsCommandBuffer::BindSubpassPipeline(PGraphicsPipeline pipeline)
	{
		auto& subpass = p_OwningInstance->p_DXRenderPass->m_D3D12Subpasses[m_SubpassId];
		auto find = subpass.pipelineInstances.find(pipeline);
		CRIM_ASSERT_AND_RETURN_VOID(find != subpass.pipelineInstances.end(), "D3D12 BindSubpassPipeline Issue! pipeline instance not found!");
		m_CommandList->SetPipelineState(find->second.Get());
	}
	void D3D12GraphicsCommandBuffer::BindVertexInputeBuffer(std::vector<PGPUBuffer> const& buffer_list, std::vector<BufferRange> const& buffer_range_list, std::vector<uint64_t> const& vertex_strides)
	{
		std::vector<D3D12_VERTEX_BUFFER_VIEW> bufferViews(buffer_list.size());
		for (uint32_t id = 0; id < buffer_list.size(); ++id)
		{
			D3D12BufferObject* dxbuffer = static_cast<D3D12BufferObject*>(buffer_list[id]);
			auto& buffer_view = bufferViews[id];
			buffer_view.BufferLocation = dxbuffer->m_Buffer->GetGPUVirtualAddress() + buffer_range_list[id].m_Offset;
			buffer_view.SizeInBytes = buffer_range_list[id].m_Size;
			buffer_view.StrideInBytes = vertex_strides[id];
		}
		m_CommandList->IASetVertexBuffers(0, buffer_list.size(), bufferViews.data());
	}
	void D3D12GraphicsCommandBuffer::BindIndexBuffer(PGPUBuffer buffer, BufferRange const& buffer_range, EIndexType index_type)
	{
		D3D12BufferObject* dxbuffer = static_cast<D3D12BufferObject*>(buffer);
		D3D12_INDEX_BUFFER_VIEW index_buffer_view{};
		index_buffer_view.BufferLocation = dxbuffer->m_Buffer->GetGPUVirtualAddress() + buffer_range.m_Offset;
		index_buffer_view.Format = index_type == EIndexType::E_INDEX_TYPE_16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
		index_buffer_view.SizeInBytes = buffer_range.m_Size;
		m_CommandList->IASetIndexBuffer(&index_buffer_view);
	}
	void D3D12GraphicsCommandBuffer::DrawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, uint32_t first_vertex, uint32_t first_instance_id)
	{
		m_CommandList->DrawIndexedInstanced(index_count, instance_count, first_index, first_vertex, first_instance_id);
	}
	void D3D12GraphicsCommandBuffer::Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance_id)
	{
		m_CommandList->DrawInstanced(vertex_count, instance_count, first_vertex, first_instance_id);
	}
	void D3D12GraphicsCommandBuffer::InitCommandBuffer(D3D12GPUDeviceThread* owningThread, ComPtr<ID3D12GraphicsCommandList4> cmdList, D3D12RenderPassInstance* renderpass_instance, uint32_t subpass)
	{
		p_OwningThread = owningThread;
		m_CommandList = cmdList;
		p_OwningInstance = renderpass_instance;
		m_SubpassId = subpass;
	}
}