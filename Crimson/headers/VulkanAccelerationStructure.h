#pragma once
#include <headers/VulkanGPUDevice.h>
#include <include/RayTraceGeometry.h>

namespace Crimson
{
	class VulkanBufferObject;
	class VulkanAccelerationStructure : public AccelerationStructure
	{
	public:
		virtual void InitAS() override;
	private:
		VulkanGPUDevice* p_OwningDevice;
		VkAccelerationStructureNV m_Structure;
		VulkanBufferObject* p_ScratchBuffer;
		VmaAllocation m_Allocation;
		VmaAllocationInfo m_AllocationInfo;
	};
}