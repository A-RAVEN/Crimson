#pragma once
#include <private/include/VulkanIncludes.h>
#include <VulkanMemoryAllocator/include/vk_mem_alloc.h>
#include <private/include/VulkanApplicationSubobjectBase.h>
#include <RenderInterface/header/Common.h>

namespace graphics_backend
{
	struct CVulkanBufferObject
	{
	public:
		vk::Buffer GetBuffer() const { return m_Buffer; }
		VmaAllocationInfo const& GetAllocationInfo() const { return m_BufferAllocationInfo; }
		void* GetMappedPointer() const;
	private:
		vk::Buffer m_Buffer = nullptr;
		VmaAllocation m_BufferAllocation = nullptr;
		VmaAllocationInfo m_BufferAllocationInfo {};
		TIndex m_OwningFrameBoundPoolId = INVALID_INDEX;
		friend class CVulkanMemoryManager;
		friend class CFrameBoundMemoryPool;
		friend class CGlobalMemoryPool;
		friend class GPUBuffer_Impl;
	};
}
