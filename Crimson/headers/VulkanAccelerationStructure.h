#pragma once
#include <headers/VulkanGPUDevice.h>
#include <include/RayTraceGeometry.h>

namespace Crimson
{
	class VulkanBufferObject;
	class VulkanAccelerationStructure : public AccelerationStructure
	{
	public:
		friend class VulkanExecutionCommandBuffer;
		friend class VulkanDescriptorSet;
		VulkanAccelerationStructure(VulkanGPUDevice* device);
		virtual void Dispose() override;
		virtual void InitAS(bool top_level) override;
		virtual void SetupScratchBuffer() override;
		virtual void FreeScratchBuffer() override;
		virtual uint64_t GetHandle() override;
		virtual PGPUBuffer GetScratchBuffer() override;
	private:
		VulkanGPUDevice* p_OwningDevice;

		VkAccelerationStructureNV m_Structure;
		VulkanBufferObject* p_ScratchBuffer;
		VmaAllocation m_Allocation;
		VmaAllocationInfo m_AllocationInfo;
		VkAccelerationStructureInfoNV m_StructureInfo;
		std::vector<VkGeometryNV> p_Geometries;

		uint64_t m_Handle;
	};
}