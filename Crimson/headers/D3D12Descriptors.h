#pragma once
#include <include/Pipeline.h>
#include <headers/D3D12GPUDevice.h>

namespace Crimson
{
	class D3D12DescriptorSet : public DescriptorSet
	{
	public:
		virtual void WriteDescriptorSetBuffers(uint32_t binding_point,
			std::vector<PGPUBuffer> const& buffers, std::vector<BufferRange> const& buffer_ranges, uint32_t start_array_id) = 0;
		virtual void WriteDescriptorSetTexelBufferView(uint32_t binding_point, PGPUBuffer buffer, std::string const& view_name, uint32_t array_id) = 0;
		virtual void WriteDescriptorSetImage(uint32_t binding_point,
			PGPUImage image, EFilterMode filter_mode, EAddrMode addr_mode, EViewAsType view_as = EViewAsType::E_VIEW_AS_TYPE_MAX, uint32_t array_id = 0) = 0;
		virtual void WriteDescriptorSetAccelStructuresNV(uint32_t binding_point,
			std::vector<PAccelerationStructure> const& structures) = 0;
		virtual void EndWriteDescriptorSet() {};
	private:
		std::vector<D3D12DescriptorHeapWrapper::DescriptorRange> m_Ranges;
	};

	class D3D12DescriptorSetLayout : public DescriptorSetLayout
	{
	public:
		virtual PDescriptorSet AllocDescriptorSet() = 0;
		virtual void BuildLayout() = 0;
	};
}