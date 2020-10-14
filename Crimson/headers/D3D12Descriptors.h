#pragma once
#include <include/Pipeline.h>
#include <headers/D3D12GPUDevice.h>
#include <headers/D3D12Buffer.h>

namespace Crimson
{
	class D3D12DescriptorSetLayout;
	class D3D12DescriptorSet : public DescriptorSet
	{
	public:
		virtual void WriteDescriptorSetBuffers(uint32_t binding_point,
			std::vector<PGPUBuffer> const& buffers, std::vector<BufferRange> const& buffer_ranges, uint32_t start_array_id);
		virtual void WriteDescriptorSetTexelBufferView(uint32_t binding_point, PGPUBuffer buffer, std::string const& view_name, uint32_t array_id);
		virtual void WriteDescriptorSetImage(uint32_t binding_point,
			PGPUImage image, EFilterMode filter_mode, EAddrMode addr_mode, EViewAsType view_as = EViewAsType::E_VIEW_AS_TYPE_MAX, uint32_t array_id = 0);
		virtual void WriteDescriptorSetAccelStructuresNV(uint32_t binding_point,
			std::vector<PAccelerationStructure> const& structures);
		virtual void EndWriteDescriptorSet() {};
		friend class D3D12DescriptorSetLayout;
	private:
		D3D12GPUDevice* p_OwningDevice;
		D3D12DescriptorSetLayout* p_OwningSetLayout;
		std::vector<D3D12DescriptorHeapWrapper::DescriptorRange> m_Ranges;
		//						heapId, handle id
		std::vector<std::pair<uint32_t, uint64_t>> m_DescriptorReference;
	};

	class D3D12DescriptorSetLayout : public DescriptorSetLayout
	{
	public:
		D3D12DescriptorSetLayout(D3D12GPUDevice* p_device)
		{
			p_OwningDevice = p_device;
		}
		virtual PDescriptorSet AllocDescriptorSet();
		virtual void BuildLayout();
	private:
		D3D12GPUDevice* p_OwningDevice;
		std::vector<std::pair<D3D12_DESCRIPTOR_HEAP_TYPE, uint32_t>> m_HeapAllocationInfo;
		std::vector<std::pair<uint32_t, uint64_t>> m_DescriptorReference;
	};
}