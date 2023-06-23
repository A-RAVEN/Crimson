#pragma once
#include <private/include/VulkanIncludes.h>

namespace graphics_backend
{
	class CVulkanBufferObject
	{
	public:
		vk::Buffer m_Buffer = nullptr;
		VmaAllocation m_BufferAllocation = nullptr;
	};
}