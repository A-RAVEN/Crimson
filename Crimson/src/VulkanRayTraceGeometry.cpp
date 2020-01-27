#include <headers/VulkanRayTraceGeometry.h>
#include <headers/VulkanTranslator.h>
#include <headers/VulkanBuffer.h>

namespace Crimson
{
	void VulkanRayTraceGeometry::BuildGeometry()
	{
		VkGeometryDataNV geometry_data{};
	}
	void VulkanRayTraceGeometry::Dispose()
	{

	}
	VulkanRayTraceGeometry::VulkanRayTraceGeometry() :
		m_GeometryData{}
	{
		m_GeometryData.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
		m_GeometryData.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
	}
	void VulkanRayTraceGeometry::SetVertexBuffer(PGPUBuffer vertex_buffer)
	{
		m_GeometryData.triangles.vertexData = static_cast<VulkanBufferObject*>(vertex_buffer)->GetVulkanBuffer();
	}
	void VulkanRayTraceGeometry::SetVertexBufferOffset(uint64_t offset)
	{
		m_GeometryData.triangles.vertexOffset = offset;
	}
	void VulkanRayTraceGeometry::SetVertexCount(uint32_t count)
	{
		m_GeometryData.triangles.vertexCount = count;
	}
	void VulkanRayTraceGeometry::SetVertexStride(uint64_t stride)
	{
		m_GeometryData.triangles.vertexStride = stride;
	}
	void VulkanRayTraceGeometry::SetVertexDataType(EDataType data_type)
	{
		m_GeometryData.triangles.vertexFormat = TranslateDataTypeToVulkanVertexInputDataTypeInfo(data_type).m_Format;
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
		m_GeometryData.triangles.indexData = static_cast<VulkanBufferObject*>(index_buffer)->GetVulkanBuffer();
	}
	void VulkanRayTraceGeometry::SetIndexBufferOffset(uint32_t offset)
	{
		m_GeometryData.triangles.indexOffset = offset;
	}
	void VulkanRayTraceGeometry::SetIndexCount(uint32_t count)
	{
		m_GeometryData.triangles.indexCount = count;
	}
	void VulkanRayTraceGeometry::SetIndexType(EIndexType index_type)
	{
		m_GeometryData.triangles.indexType = (index_type == EIndexType::E_INDEX_TYPE_32) ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16;
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
		m_GeometryData.aabbs.aabbData = static_cast<VulkanBufferObject*>(aabb_buffer)->GetVulkanBuffer();
		m_GeometryData.aabbs.numAABBs = aabb_count;
		m_GeometryData.aabbs.stride = stride;
		m_GeometryData.aabbs.offset = offset;
		m_GeometryData.aabbs.pNext = nullptr;
	}
}