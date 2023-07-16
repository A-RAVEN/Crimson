#pragma once
#include "Common.h"
#include "CVertexInputDescriptor.h"
#include <vector>

namespace graphics_backend
{


	class CGPUPrimitiveResource
	{
	public:
		CGPUPrimitiveResource() = default;
		CGPUPrimitiveResource(CGPUPrimitiveResource const& other) = delete;
		CGPUPrimitiveResource& operator=(CGPUPrimitiveResource const& other) = delete;
		CGPUPrimitiveResource(CGPUPrimitiveResource && other) = default;
		CGPUPrimitiveResource& operator=(CGPUPrimitiveResource && other) = default;
		virtual ~CGPUPrimitiveResource() {};
		//virtual void AddPrimitiveDescriptor(
		//	uint32_t stride
		//	,std::vector<VertexAttribute> const& attributes
		//	,bool perInstance = false
		//) = 0;
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

	public:
		CVertexInputDescriptor m_VertexInputDescriptor = {};
	};
}