#include <headers/D3D12Descriptors.h>
#include <headers/D3D12Translator.h>

namespace Crimson
{
	void D3D12DescriptorSet::WriteDescriptorSetBuffers(uint32_t binding_point, std::vector<PGPUBuffer> const& buffers, std::vector<BufferRange> const& buffer_ranges, uint32_t start_array_id)
	{
		auto& reference = m_DescriptorReference[binding_point];
		uint32_t index = start_array_id;
		for (uint32_t id = 0; id < buffers.size(); ++id)
		{
			auto buffer = buffers[id];
			BufferRange const& range = buffer_ranges[id];
			auto handle = m_Ranges[reference.first][reference.second + index];
			D3D12BufferObject* d3d12buffer = static_cast<D3D12BufferObject*>(buffer);
			D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
			desc.ViewDimension = D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_BUFFER;
			desc.Buffer.FirstElement = range.m_Offset;
			desc.Buffer.NumElements = range.m_Size;
			desc.Buffer.StructureByteStride = 1;
			desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
			p_OwningDevice->m_Device->CreateShaderResourceView(d3d12buffer->m_Buffer.Get(), &desc, handle);
			++index;
		}
	};
	void D3D12DescriptorSet::WriteDescriptorSetTexelBufferView(uint32_t binding_point, PGPUBuffer buffer, std::string const& view_name, uint32_t array_id)
	{
	}
	void D3D12DescriptorSet::WriteDescriptorSetImage(uint32_t binding_point, PGPUImage image, EFilterMode filter_mode, EAddrMode addr_mode, EViewAsType view_as, uint32_t array_id)
	{

	}
	void D3D12DescriptorSet::WriteDescriptorSetAccelStructuresNV(uint32_t binding_point, std::vector<PAccelerationStructure> const& structures)
	{
	}
	PDescriptorSet D3D12DescriptorSetLayout::AllocDescriptorSet()
	{
		D3D12DescriptorSet* return_val = new D3D12DescriptorSet();
		return_val->p_OwningSetLayout = this;
		return_val->p_OwningDevice = p_OwningDevice;
		return_val->m_DescriptorReference = m_DescriptorReference;
		return_val->m_Ranges.resize(m_HeapAllocationInfo.size());
		for (uint32_t i = 0; i < m_HeapAllocationInfo.size(); ++i)
		{
			auto range = p_OwningDevice->m_DescriptorHeaps.m_Heaps[m_HeapAllocationInfo[i].first].AllocDescriptor(m_HeapAllocationInfo[i].second);
			return_val->m_Ranges[i] = range;
		}
		return return_val;
	}
	void D3D12DescriptorSetLayout::BuildLayout()
	{
		std::map<D3D12_DESCRIPTOR_HEAP_TYPE, std::vector<uint32_t>> heaptype_DescriptorsMap;
		for (uint32_t i = 0; i < m_Bindings.size(); ++i)
		{
			auto& binding = m_Bindings[i];
			auto& info = D3D12ResourceTypeInfo(binding.m_ResourceType);
			auto find = heaptype_DescriptorsMap.find(info.descriptorHeapType);
			if (find == heaptype_DescriptorsMap.end())
			{
				heaptype_DescriptorsMap.insert(std::make_pair(info.descriptorHeapType, std::vector<uint32_t>{i}));
			}
			else
			{
				find->second.push_back(i);
			}
		}

		m_HeapAllocationInfo.clear();
		m_DescriptorReference.resize(m_Bindings.size());
		for (auto& pair : heaptype_DescriptorsMap)
		{
			uint32_t heapId = m_HeapAllocationInfo.size();
			uint64_t heapSize = 0;
			for (auto bindingId : pair.second)
			{
				m_DescriptorReference[bindingId] = std::make_pair(heapId, heapSize);
				heapSize += m_Bindings[bindingId].m_Num;
			}
			m_HeapAllocationInfo.push_back(std::make_pair(pair.first, heapSize));
		}
	}
}