#pragma once
#include <include/RayTraceGeometry.h>
#include <headers/VulkanGPUDevice.h>

namespace Crimson
{
	class VulkanRayTraceGeometry : public RayTraceGeometry
	{
	public:
		virtual void Dispose() override;
		VulkanRayTraceGeometry(VulkanGPUDevice *device);
		virtual void SetVertexBuffer(PGPUBuffer vertex_buffer) override;
		virtual void SetVertexBufferOffset(uint64_t offset) override;
		virtual void SetVertexCount(uint32_t count) override;
		virtual void SetVertexStride(uint64_t stride) override;
		virtual void SetVertexDataType(EDataType data_type) override;
		virtual void SetVertexData(PGPUBuffer vertex_buffer, 
			uint64_t offset, uint32_t count, uint64_t stride, EDataType data_type);

		virtual void SetIndexBuffer(PGPUBuffer index_buffer) override;
		virtual void SetIndexBufferOffset(uint32_t offset) override;
		virtual void SetIndexCount(uint32_t count) override;
		virtual void SetIndexType(EIndexType index_type) override;
		virtual void SetIndexData(PGPUBuffer index_buffer, uint32_t offset, uint32_t count, EIndexType index_type) override;

		virtual void SetAABBData(PGPUBuffer aabb_buffer, uint32_t aabb_count, uint32_t stride, uint64_t offset) override;

		virtual void SetGeometryType(ERayTraceGeometryType type) override;
		virtual void SetGeometryFlags(std::vector<EGeometryFlags> const& flags) override;

		VkGeometryNV const& GetGeometry() { return m_Geometry; }
	private:
		VulkanGPUDevice* p_OwningDevice;
		VkGeometryNV m_Geometry;
	};
}