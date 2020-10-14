#pragma once
#include "Generals.h"
#include "PipelineEnums.h"
#include "RayTraceGeometry.h"

namespace Crimson
{
	class DescriptorSet : IObject
	{
	public:
		//virtual void WriteDescriptorSetBuffer(uint32_t binding_point, EBufferUniformType uniform_type, PGPUBuffer buffer, uint64_t buffer_offset, uint64_t write_size) {};
		virtual void WriteDescriptorSetBuffers(uint32_t binding_point,
			std::vector<PGPUBuffer> const& buffers, std::vector<BufferRange> const& buffer_ranges, uint32_t start_array_id) = 0;
		virtual void WriteDescriptorSetTexelBufferView(uint32_t binding_point, PGPUBuffer buffer, std::string const& view_name, uint32_t array_id) = 0;
		virtual void WriteDescriptorSetImage(uint32_t binding_point,
			PGPUImage image, EFilterMode filter_mode, EAddrMode addr_mode, EViewAsType view_as = EViewAsType::E_VIEW_AS_TYPE_MAX, uint32_t array_id = 0) = 0;
		virtual void WriteDescriptorSetAccelStructuresNV(uint32_t binding_point,
			std::vector<PAccelerationStructure> const& structures) = 0;
		virtual void EndWriteDescriptorSet() {};
	};
	using PDescriptorSet = DescriptorSet*;

	class ShaderBinding
	{
	public:
		uint32_t					m_BindingPoint;	//	meaning binding point for vulkan and space for dx12
		uint32_t					m_RegisterPoint; // intended for dx12 api, similar to binding point of vulkan but type local
		EShaderResourceType			m_ResourceType;
		std::vector<EShaderType>	m_ShaderTypes;
		uint32_t					m_Num;
		ShaderBinding(uint32_t bindpoint = 0, uint32_t registerpoint = 0, EShaderResourceType resource_type = EShaderResourceType::E_SHADER_RESOURCE_TYPE_MAX, std::vector<EShaderType> const& shader_types = {}, uint32_t array_number = 1) :
			m_BindingPoint(bindpoint),
			m_RegisterPoint(registerpoint),
			m_ResourceType(resource_type),
			m_ShaderTypes(shader_types),
			m_Num(array_number)
		{}
	};

	class DescriptorSetLayout : IObject
	{
	public:
		virtual PDescriptorSet AllocDescriptorSet() = 0;
		virtual void BuildLayout() = 0;
		std::vector<ShaderBinding> m_Bindings;
	};
	using PDescriptorSetLayout = DescriptorSetLayout*;
}