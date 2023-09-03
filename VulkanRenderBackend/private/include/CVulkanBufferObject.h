#pragma once
#include <private/include/VulkanIncludes.h>
#include <VulkanMemoryAllocator/include/vk_mem_alloc.h>
#include <private/include/VulkanApplicationSubobjectBase.h>
#include <RenderInterface/header/Common.h>

namespace graphics_backend
{
	struct CVulkanBufferObject : public BaseApplicationSubobject
	{
	public:
		CVulkanBufferObject(CVulkanApplication & owner);
		void Initialize(vk::Buffer const& buffer, VmaAllocation const& allocation, VmaAllocationInfo const& allocationInfo);
		virtual void Release() override;
		vk::Buffer GetBuffer() const { return m_Buffer; }
		VmaAllocationInfo const& GetAllocationInfo() const { return m_BufferAllocationInfo; }
		void* GetMappedPointer() const;
		const bool operator==(CVulkanBufferObject const& rhs) const { return m_Buffer == rhs.m_Buffer; }
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
