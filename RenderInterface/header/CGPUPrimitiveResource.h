#pragma once
#include "Common.h"
#include <vector>

namespace graphics_backend
{
	struct VertexAttribute
	{
	public:
		uint32_t attributeIndex;
		uint32_t offset;
		uint32_t size;
	};

	class CGPUPrimitiveResource
	{
	public:
		CGPUPrimitiveResource(CGPUPrimitiveResource const& other) = delete;
		CGPUPrimitiveResource(CGPUPrimitiveResource && other) = default;
		virtual void AddPrimitiveDescriptor(
			uint32_t stride
			,std::vector<VertexAttribute> const& attributes
			,bool perInstance = false
		) = 0;
		virtual void SetPrimitiveData(
			uint32_t primitiveIndex
			, uint32_t dataSize
			, void* data
		) = 0;
		virtual void SetIndexData(
			uint32_t dataSize
			, void* data
			, bool bit16Index = false
		) = 0;

		virtual void Submit() = 0;
		virtual bool GPUDone() = 0;
	};
}