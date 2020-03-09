#pragma once
#include "Generals.h"
#include "PipelineEnums.h"

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

	//Geometry Instance Flags Directly Copied From VkGeometryInstanceFlagBits
	enum class EGeometryInstanceFlags : uint32_t
	{
		E_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE = 0x00000001,
		E_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE = 0x00000002,
		E_GEOMETRY_INSTANCE_FORCE_OPAQUE = 0x00000004,
		E_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE = 0x00000008,
		E_GEOMETRY_INSTANCE_MAX = 0x7FFFFFFF
	};

	class RayTraceGeometry : public IObject
	{
	public:
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

	struct RayTraceGeometryInstanceOLD
	{
	public:
		float m_TransformMatrix[12];
		uint32_t m_InstanceId : 24;
		uint32_t m_Mask : 8;
		uint32_t m_InstanceOffset : 24;
		uint32_t m_Flags : 8;
		uint64_t m_AccelerationStructureHandle;
	};

	template <typename MATTYPE>
	struct RayTraceGeometryInstance
	{
		static_assert(sizeof(MATTYPE) == sizeof(float) * 12, "Type of MATTYPE of RayTraceGeometryInstance<MATTYPE> must be a 3x4 float matrix type");
	public:
		MATTYPE m_TransformMatrix;
		uint32_t m_InstanceId : 24;
		uint32_t m_Mask : 8;
		uint32_t m_InstanceOffset : 24;
		uint32_t m_Flags : 8;
		uint64_t m_AccelerationStructureHandle;
	};

	//Geometry Group 
	class AccelerationStructure : public IObject
	{
	public:
		std::vector<PRayTraceGeometry> m_Geometries;
		std::vector<EBuildAccelerationStructureFlags> m_BuildFlags;
		uint32_t m_InstanceNumber;
		// init acceleration structure, them a build should be done in cmd buffer
		virtual void InitAS(bool top_level = false) = 0;
		virtual void SetupScratchBuffer() = 0;
		virtual void FreeScratchBuffer() = 0;
		virtual uint64_t GetHandle() = 0;
		AccelerationStructure() :
			m_InstanceNumber(0)
		{}
	};
	using PAccelerationStructure = AccelerationStructure*;

}