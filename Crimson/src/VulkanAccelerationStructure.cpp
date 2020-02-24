#include <headers/VulkanAccelerationStructure.h>
#include <headers/VulkanRayTraceGeometry.h>
#include <headers/VulkanDebugLog.h>
#include <headers/VulkanBuffer.h>
#include <headers/VulkanTranslator.h>

namespace Crimson
{
	VulkanAccelerationStructure::VulkanAccelerationStructure(VulkanGPUDevice* device) : 
		p_OwningDevice(device),
		m_Structure(VK_NULL_HANDLE),
		p_ScratchBuffer(nullptr),
		m_Allocation(VK_NULL_HANDLE),
		m_AllocationInfo(),
		m_StructureInfo()
	{}

	void VulkanAccelerationStructure::Dispose()
	{
		if (m_Structure)
		{
			p_OwningDevice->m_NVExtension.vkDestroyAccelerationStructureNV(p_OwningDevice->m_LogicalDevice, m_Structure, VULKAN_ALLOCATOR_POINTER);
			m_Structure = VK_NULL_HANDLE;
		}
		if (p_ScratchBuffer)
		{
			p_ScratchBuffer->Dispose();
			p_ScratchBuffer = nullptr;
		}
		p_OwningDevice->HandleDisposedAccelerationStructure(this);
	}

	void VulkanAccelerationStructure::InitAS(bool top_level)
	{
		VkAccelerationStructureCreateInfoNV create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
		create_info.pNext = nullptr;
		create_info.compactedSize = 0;
		m_StructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
		m_StructureInfo.geometryCount = m_Geometries.size();
		p_Geometries = std::vector<VkGeometryNV>(m_StructureInfo.geometryCount);
		for (uint32_t id = 0; id < m_StructureInfo.geometryCount; ++id)
		{
			p_Geometries[id] = static_cast<VulkanRayTraceGeometry*>(m_Geometries[id])->GetGeometry();
		}
		m_StructureInfo.pGeometries = p_Geometries.data();
		m_StructureInfo.flags = TranslateBuildAccelStructureFlags(m_BuildFlags);
		m_StructureInfo.type = top_level ? VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV : VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
		m_StructureInfo.instanceCount = m_InstanceNumber;
		create_info.info = m_StructureInfo;

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
		CHECK_VKRESULT(p_OwningDevice->m_NVExtension.vkBindAccelerationStructureMemoryNV(p_OwningDevice->m_LogicalDevice, 1, &bind_info),
			"Vulkan Bind Acceleration Structure Issue!");
		CHECK_VKRESULT(p_OwningDevice->m_NVExtension.vkGetAccelerationStructureHandleNV(p_OwningDevice->m_LogicalDevice, m_Structure, sizeof(uint64_t), &m_Handle),
			"Vulkan Get Acceleration Struct Handle Issue!");
	}
	uint64_t VulkanAccelerationStructure::GetHandle()
	{
		return m_Handle;
	}
}