#include <headers/VulkanRayTraceGeometry.h>
#include <headers/VulkanTranslator.h>
#include <headers/VulkanBuffer.h>

namespace Crimson
{
	void VulkanRayTraceGeometry::Dispose()
	{
		p_OwningDevice->HandleDisposedRayTraceGeometry(this);
	}
	VulkanRayTraceGeometry::VulkanRayTraceGeometry(VulkanGPUDevice* device) :
		p_OwningDevice(device),
		m_Geometry{}
	{
		m_Geometry.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
		m_Geometry.pNext = nullptr;
		m_Geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
		m_Geometry.geometry.triangles.transformData = VK_NULL_HANDLE;
		m_Geometry.geometry.triangles.transformOffset = 0;
		m_Geometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
		m_Geometry.flags = 0;
	}
	void VulkanRayTraceGeometry::SetVertexBuffer(PGPUBuffer vertex_buffer)
	{
		m_Geometry.geometry.triangles.vertexData = static_cast<VulkanBufferObject*>(vertex_buffer)->GetVulkanBuffer();
	}
	void VulkanRayTraceGeometry::SetVertexBufferOffset(uint64_t offset)
	{
		m_Geometry.geometry.triangles.vertexOffset = offset;
	}
	void VulkanRayTraceGeometry::SetVertexCount(uint32_t count)
	{
		m_Geometry.geometry.triangles.vertexCount = count;
	}
	void VulkanRayTraceGeometry::SetVertexStride(uint64_t stride)
	{
		m_Geometry.geometry.triangles.vertexStride = stride;
	}
	void VulkanRayTraceGeometry::SetVertexDataType(EDataType data_type)
	{
		m_Geometry.geometry.triangles.vertexFormat = TranslateDataTypeToVulkanVertexInputDataTypeInfo(data_type).m_Format;
	}
	void VulkanRayTraceGeometry::SetVertexData(PGPUBuffer vertex_buffer, uint64_t offset, uint32_t count, uint64_t stride, EDataType data_type)
	{
		SetVertexBuffer(vertex_buffer);
		SetVertexBufferOffset( offset);
		SetVertexCount(count);
		SetVertexStride(stride);
		SetVertexDataType(data_type);
	}
	void VulkanRayTraceGeometry::SetIndexBuffer(PGPUBuffer index_buffer)
	{
		m_Geometry.geometry.triangles.indexData = static_cast<VulkanBufferObject*>(index_buffer)->GetVulkanBuffer();
	}
	void VulkanRayTraceGeometry::SetIndexBufferOffset(uint32_t offset)
	{
		m_Geometry.geometry.triangles.indexOffset = offset;
	}
	void VulkanRayTraceGeometry::SetIndexCount(uint32_t count)
	{
		m_Geometry.geometry.triangles.indexCount = count;
	}
	void VulkanRayTraceGeometry::SetIndexType(EIndexType index_type)
	{
		m_Geometry.geometry.triangles.indexType = (index_type == EIndexType::E_INDEX_TYPE_32) ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16;
	}
	void VulkanRayTraceGeometry::SetIndexData(PGPUBuffer index_buffer, uint32_t offset, uint32_t count, EIndexType index_type)
	{
		SetIndexBuffer(index_buffer);
		SetIndexBufferOffset(offset);
		SetIndexCount(count);
		SetIndexType(index_type);
	}
	void VulkanRayTraceGeometry::SetAABBData(PGPUBuffer aabb_buffer, uint32_t aabb_count, uint32_t stride, uint64_t offset)
	{
		m_Geometry.geometry.aabbs.aabbData = static_cast<VulkanBufferObject*>(aabb_buffer)->GetVulkanBuffer();
		m_Geometry.geometry.aabbs.numAABBs = aabb_count;
		m_Geometry.geometry.aabbs.stride = stride;
		m_Geometry.geometry.aabbs.offset = offset;
		m_Geometry.geometry.aabbs.pNext = nullptr;
	}
	void VulkanRayTraceGeometry::SetGeometryType(ERayTraceGeometryType type)
	{
		switch (type)
		{
		case ERayTraceGeometryType::E_GEOMETRY_TYPE_TRIANGLES:
			m_Geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
			break;
		case ERayTraceGeometryType::E_GEOMETRY_TYPE_AABBS:
			m_Geometry.geometryType = VK_GEOMETRY_TYPE_AABBS_NV;
			break;
		}
	}
	void VulkanRayTraceGeometry::SetGeometryFlags(std::vector<EGeometryFlags> const& flags)
	{
		m_Geometry.flags = 0;
		for (auto flag : flags)
		{
			m_Geometry.flags |= TranslateGeometryFlagToVulkan(flag);
		}
	}
}