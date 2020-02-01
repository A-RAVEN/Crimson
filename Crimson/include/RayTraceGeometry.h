#pragma once
#include "Pipeline.h"

namespace Crimson
{
	enum class EBuildAccelerationStructureFlags : uint8_t
	{
		E_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV = 0,
		E_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_NV,
		E_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV,
		E_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV,
		E_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_NV,
		E_BUILD_ACCELERATION_STRUCTURE_MAX,
	};

	enum class ERayTraceGeometryType : uint8_t
	{
		E_GEOMETRY_TYPE_TRIANGLES = 0,
		E_GEOMETRY_TYPE_AABBS,
		E_GEOMETRY_TYPE_MAX
	};

	enum class EGeometryFlags : uint8_t
	{
		E_GEOMETRY_OPAQUE = 0,
		E_GEOMETRY_NO_DUPLICATE_ANYHIT,
		E_GEOMETRY_MAX,
	};

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

		virtual void SetGeometryType(ERayTraceGeometryType) = 0;
		virtual void SetGeometryFlags(std::vector<EGeometryFlags> const& flags) = 0;
	};
	using PRayTraceGeometry = RayTraceGeometry*;

	//Geometry Group 
	class AccelerationStructure : public IObject
	{
	public:
		std::vector<PRayTraceGeometry> m_Geometries;
		std::vector<EBuildAccelerationStructureFlags> m_BuildFlags;
		// init acceleration structure, them a build should be done in cmd buffer
		virtual void InitAS() = 0;
	};
	using PAccelerationStructure = AccelerationStructure*;

}