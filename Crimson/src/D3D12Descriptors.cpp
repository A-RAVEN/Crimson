#include <headers/D3D12Descriptors.h>

namespace Crimson
{
	void D3D12DescriptorSet::WriteDescriptorSetTexelBufferView(uint32_t binding_point, PGPUBuffer buffer, std::string const& view_name, uint32_t array_id)
	{
	}
	void D3D12DescriptorSet::WriteDescriptorSetImage(uint32_t binding_point, PGPUImage image, EFilterMode filter_mode, EAddrMode addr_mode, EViewAsType view_as, uint32_t array_id)
	{

	}
	void D3D12DescriptorSet::WriteDescriptorSetAccelStructuresNV(uint32_t binding_point, std::vector<PAccelerationStructure> const& structures)
	{
	}
}