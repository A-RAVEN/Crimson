#include <private/include/pch.h>
#include <private/include/CVulkanBufferObject.h>
#include <private/include/CVulkanThreadContext.h>

namespace graphics_backend
{
	void CVulkanBufferObject::Release_Internal()
	{
		CVulkanThreadContext* threadContext = GetThreadContext(m_OwningThreadContextId);
		if (threadContext != nullptr)
		{
			threadContext->ReleaseBufferObject(this);
		}
		m_Buffer = nullptr;
		m_BufferAllocation = nullptr;
		m_BufferAllocationInfo = {};
		m_OwningThreadContextId = std::numeric_limits<uint32_t>::max();
	}
}