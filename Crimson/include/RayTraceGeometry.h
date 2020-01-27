#pragma once
#include "Pipeline.h"

namespace Crimson
{


	class RayTraceGeometry : public IObject
	{
	public:
		virtual void BuildGeometry() = 0;
		virtual void SetVertexBuffer(PGPUBuffer vertex_buffer) = 0;
		virtual void SetVertexBufferOffset(uint64_t offset) = 0;
		virtual void SetVertexCount(uint32_t count) = 0;
		virtual void SetVertexStride(uint64_t stride) = 0;
		virtual void SetVertexDataType(EDataType data_type) = 0;
		virtual void SetVertexData(PGPUBuffer vertex_buffer,
			uint64_t offset, uint32_t count, uint64_t stride, EDataType data_type) = 0;

		virtual void SetIndexBuffer(PGPUBuffer index_buffer) = 0;
		virtual void SetIndexBufferOffset(uint32_t offset) = 0;
		virtual void SetIndexCount(uint32_t count) = 0;
		virtual void SetIndexType(EIndexType index_type) = 0;
		virtual void SetIndexData(PGPUBuffer index_buffer, uint32_t offset, uint32_t count, EIndexType index_type) = 0;
		virtual void SetAABBData(PGPUBuffer aabb_buffer, uint32_t aabb_count, uint32_t stride, uint64_t offset) = 0;
	};
}