#pragma once
#include <string>
#include "PipelineEnums.h"
#include "DescriptorSets.h"

namespace Crimson
{
	class RayTracer : IObject
	{
	public:
		virtual void LoadShaderSource(char const* src_code, size_t src_size, EShaderType shader_type, std::string const& shader_table) = 0;
		virtual uint64_t GetShaderTableSize(std::string const& shader_table_name) = 0;
		virtual void CopyShaderTable(void* copy_dst, std::string const& shader_table_name) = 0;
		virtual void Build() = 0;
		uint32_t m_MaxRecursionDepth;
		std::vector<std::pair<uint32_t, PDescriptorSetLayout>>		m_DescriptorSetLayouts;
	protected:
		RayTracer() :
			m_MaxRecursionDepth(0)
		{}
	};
	using PRayTracer = RayTracer*;
}