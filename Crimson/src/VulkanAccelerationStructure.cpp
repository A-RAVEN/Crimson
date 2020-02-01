#include <headers/VulkanAccelerationStructure.h>
#include <headers/VulkanRayTraceGeometry.h>
#include <headers/VulkanDebugLog.h>
#include <headers/VulkanBuffer.h>

namespace Crimson
{
	void VulkanAccelerationStructure::InitAS()
	{
		VkAccelerationStructureCreateInfoNV create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
		create_info.pNext = nullptr;
		create_info.compactedSize = 0;
		create_info.info.geometryCount = m_Geometries.size();
		std::vector<VkGeometryNV> geometries(create_info.info.geometryCount);
		for (uint32_t id = 0; id < create_info.info.geometryCount; ++id)
		{
			geometries[id] = static_cast<VulkanRayTraceGeometry*>(m_Geometries[id])->GetGeometry();
		}
		create_info.info.pGeometries = geometries.data();
		create_info.info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV | VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_NV;
		create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
		create_info.info.instanceCount = 0;

		CHECK_VKRESULT(p_OwningDevice->m_NVExtension.vkCreateAccelerationStructureNV(p_OwningDevice->m_LogicalDevice, &create_info, VULKAN_ALLOCATOR_POINTER, &m_Structure), 
			"Vulkan Create Nvidia Ray Tracing Acceleration Structure Issue!");

		{
			VkAccelerationStructureMemoryRequirementsInfoNV memory_req{};
			memory_req.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
			memory_req.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
			memory_req.accelerationStructure = m_Structure;
			memory_req.pNext = nullptr;
			VkMemoryRequirements2 scratch_mem_requirement{};
			scratch_mem_requirement.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
			scratch_mem_requirement.pNext = nullptr;
			p_OwningDevice->m_NVExtension.vkGetAccelerationStructureMemoryRequirementsNV(p_OwningDevice->m_LogicalDevice, &memory_req, &scratch_mem_requirement);
			p_ScratchBuffer = static_cast<VulkanBufferObject*>(p_OwningDevice->CreateBuffer(scratch_mem_requirement.memoryRequirements.size, { EBufferUsage::E_BUFFER_USAGE_RAYTRACING_NV }, EMemoryType::E_MEMORY_TYPE_DEVICE));
		}

		{
			VkAccelerationStructureMemoryRequirementsInfoNV memory_req{};
			memory_req.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
			memory_req.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
			memory_req.accelerationStructure = m_Structure;
			memory_req.pNext = nullptr;
			VkMemoryRequirements2 mem_requirement{};
			mem_requirement.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
			mem_requirement.pNext = nullptr;
			p_OwningDevice->m_NVExtension.vkGetAccelerationStructureMemoryRequirementsNV(p_OwningDevice->m_LogicalDevice, &memory_req, &mem_requirement);
			
			VmaAllocationCreateInfo allocation_create_info{};
			allocation_create_info.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
			allocation_create_info.memoryTypeBits = mem_requirement.memoryRequirements.memoryTypeBits;
			allocation_create_info.pool = VK_NULL_HANDLE;
			allocation_create_info.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			allocation_create_info.pUserData = nullptr;
			allocation_create_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			allocation_create_info.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
			CHECK_VKRESULT(vmaAllocateMemory(p_OwningDevice->m_MemoryAllocator, &mem_requirement.memoryRequirements, &allocation_create_info, &m_Allocation, &m_AllocationInfo),
				"VMA Allocating Acceleration Struct Memory Issue!");
		}
		VkBindAccelerationStructureMemoryInfoNV bind_info{};
		bind_info.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
		bind_info.accelerationStructure = m_Structure;
		bind_info.memory = m_AllocationInfo.deviceMemory;
		bind_info.memoryOffset = m_AllocationInfo.offset;
		vkBindAccelerationStructureMemoryNV(p_OwningDevice->m_LogicalDevice, 1, &bind_info);
	}
}