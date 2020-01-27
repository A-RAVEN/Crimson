#pragma once
#include "Pipeline.h"

namespace Crimson
{
	class RayTracer : IObject
	{
	public:
		virtual void LoadShaderSource(char const* src_code, size_t src_size, EShaderType shader_type) = 0;
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