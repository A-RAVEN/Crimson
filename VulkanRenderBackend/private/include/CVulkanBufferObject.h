#pragma once
#include <private/include/VulkanIncludes.h>
#include <VulkanMemoryAllocator/include/vk_mem_alloc.h>
#include <private/include/VulkanApplicationSubobjectBase.h>

namespace graphics_backend
{
	class CVulkanBufferObject : public ApplicationSubobjectBase
	{
	protected:
		void Initialize_Internal(CVulkanApplication const* owningApplication) override {};
		void Release_Internal() override;
	public:
		vk::Buffer m_Buffer = nullptr;
		VmaAllocation m_BufferAllocation = nullptr;
		VmaAllocationInfo m_BufferAllocationInfo;
		uint32_t m_OwningThreadContextId = std::numeric_limits<uint32_t>::max();
		friend class CVulkanThreadContext;
	};
}