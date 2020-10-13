#pragma once
#include <include/Pipeline.h>
#include <headers/D3D12GPUDevice.h>
#include <headers/D3D12Buffer.h>

namespace Crimson
{
	class D3D12DescriptorSet : public DescriptorSet
	{
	public:
		virtual void WriteDescriptorSetBuffers(uint32_t binding_point,
			std::vector<PGPUBuffer> const& buffers, std::vector<BufferRange> const& buffer_ranges, uint32_t start_array_id)
		{
			auto& reference = m_DescriptorReference[binding_point];
			uint32_t index = start_array_id;
			for (auto& buffer : buffers)
			{
				auto handle = m_Ranges[reference.first][reference.second + index];
				D3D12BufferObject* d3d12buffer = static_cast<D3D12BufferObject*>(buffer);
				D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
				//desc.Buffer.
				p_OwningDevice->m_Device->CreateShaderResourceView(d3d12buffer->m_Buffer.Get(), )
				//++index;
			}
		};
		virtual void WriteDescriptorSetTexelBufferView(uint32_t binding_point, PGPUBuffer buffer, std::string const& view_name, uint32_t array_id);
		virtual void WriteDescriptorSetImage(uint32_t binding_point,
			PGPUImage image, EFilterMode filter_mode, EAddrMode addr_mode, EViewAsType view_as = EViewAsType::E_VIEW_AS_TYPE_MAX, uint32_t array_id = 0);
		virtual void WriteDescriptorSetAccelStructuresNV(uint32_t binding_point,
			std::vector<PAccelerationStructure> const& structures);
		virtual void EndWriteDescriptorSet() {};
	private:
		D3D12GPUDevice* p_OwningDevice;
		std::vector<D3D12DescriptorHeapWrapper::DescriptorRange> m_Ranges;
		std::vector<std::pair<uint32_t, uint64_t>> m_DescriptorReference;
		std::vector<UINT64> m_Offsets;
	};

	class D3D12DescriptorSetLayout : public DescriptorSetLayout
	{
	public:
		virtual PDescriptorSet AllocDescriptorSet() = 0;
		virtual void BuildLayout() = 0;
	private:
	};
}