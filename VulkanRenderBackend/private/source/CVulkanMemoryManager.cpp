#include <private/include/pch.h>
#include <private/include/CVulkanMemoryManager.h>

#include "private/include/CVulkanApplication.h"

namespace graphics_backend
{
	void AllocateBuffer_Common(VmaAllocator allocator, EMemoryType memoryType, size_t bufferSize, vk::BufferUsageFlags bufferUsage
		, vk::Buffer& outBuffer, VmaAllocation& outAllocation, VmaAllocationInfo& outAllocationInfo)
	{
		VkBufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo(
			{}, bufferSize, bufferUsage, vk::SharingMode::eExclusive
		);
		VmaAllocationCreateInfo allocationCreateInfo{};
		switch (memoryType)
		{
		case EMemoryType::GPU:
		{
			allocationCreateInfo.flags = 0;
			allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		}
		break;
		case EMemoryType::CPU_Random_Access:
		{
			allocationCreateInfo.flags =
				VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
				| VMA_ALLOCATION_CREATE_MAPPED_BIT;
			allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
		}
		break;
		case EMemoryType::CPU_Sequential_Access:
		{
			allocationCreateInfo.flags =
				VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
				| VMA_ALLOCATION_CREATE_MAPPED_BIT;
			allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
		}
		break;
		}
		VkBuffer vkbuffer_c;
		vmaCreateBuffer(
			allocator
			, &bufferCreateInfo
			, &allocationCreateInfo
			, &vkbuffer_c
			, &outAllocation
			, &outAllocationInfo);
		outBuffer = vkbuffer_c;
	}

	CFrameBoundMemoryPool::CFrameBoundMemoryPool(uint32_t pool_id, CVulkanApplication& owner) :
		BaseApplicationSubobject(owner)
		, m_PoolId(pool_id)
		, m_BufferObjectPool(owner)
	{
	}

	CVulkanBufferObject* CFrameBoundMemoryPool::AllocateBuffer(
		EMemoryType memoryType
		, size_t bufferSize
		, vk::BufferUsageFlags bufferUsage)
	{
		vk::Buffer buffer{};
		VmaAllocation allocation{};
		VmaAllocationInfo allocationInfo{};
		AllocateBuffer_Common(m_BufferAllocator, memoryType, bufferSize, bufferUsage
			, buffer, allocation, allocationInfo);
		std::lock_guard<std::mutex> guard(m_Mutex);
		m_ActiveBuffers.emplace_back(std::make_tuple(buffer, allocation));
		CVulkanBufferObject* result = m_BufferObjectPool.Alloc(buffer, allocation, allocationInfo);
		return result;
	}

	void CFrameBoundMemoryPool::ReleaseBuffer(CVulkanBufferObject* returnBuffer)
	{
		m_BufferObjectPool.Release(returnBuffer);
	}

	void CFrameBoundMemoryPool::ReleaseAllBuffers()
	{
		m_BufferObjectPool.ReleaseAll();
		std::lock_guard<std::mutex> guard(m_Mutex);
		for (auto pending_release_buffer : m_ActiveBuffers)
		{
			vmaDestroyBuffer(m_BufferAllocator
				, std::get<0>(pending_release_buffer)
				, std::get<1>(pending_release_buffer));
		}
		m_ActiveBuffers.clear();
	}

	void CFrameBoundMemoryPool::Initialize()
	{
		VmaAllocatorCreateInfo vmaCreateInfo{};
		vmaCreateInfo.vulkanApiVersion = VULKAN_API_VERSION_IN_USE;
		vmaCreateInfo.physicalDevice = static_cast<VkPhysicalDevice>(GetPhysicalDevice());
		vmaCreateInfo.device = GetDevice();
		vmaCreateInfo.instance = GetInstance();
		vmaCreateAllocator(&vmaCreateInfo, &m_BufferAllocator);
	}

	void CFrameBoundMemoryPool::Release()
	{
		std::lock_guard<std::mutex> guard(m_Mutex);
		for (auto pending_release_buffer : m_ActiveBuffers)
		{
			vmaDestroyBuffer(m_BufferAllocator
				, std::get<0>(pending_release_buffer)
				, std::get<1>(pending_release_buffer));
		}
		m_ActiveBuffers.clear();
		vmaDestroyAllocator(m_BufferAllocator);
	}

	CGlobalMemoryPool::CGlobalMemoryPool(CVulkanApplication& owner) : BaseApplicationSubobject(owner)
		, m_BufferObjectPool(owner)
	{
	}

	CVulkanBufferObject* CGlobalMemoryPool::AllocateBuffer(
		EMemoryType memoryType
		, size_t bufferSize
		, vk::BufferUsageFlags bufferUsage)
	{

		vk::Buffer buffer{};
		VmaAllocation allocation{};
		VmaAllocationInfo allocationInfo{};
		AllocateBuffer_Common(m_BufferAllocator, memoryType, bufferSize, bufferUsage
			, buffer, allocation, allocationInfo);
		std::lock_guard<std::mutex> guard(m_Mutex);
		m_ActiveBuffers.insert(std::make_pair(buffer, allocation));
		CVulkanBufferObject* result = m_BufferObjectPool.Alloc(buffer, allocation, allocationInfo);
		return result;
	}

	void CGlobalMemoryPool::ReleaseBuffer(CVulkanBufferObject* returnBuffer)
	{
		std::lock_guard<std::mutex> guard(m_Mutex);
		FrameType frameId = GetFrameCountContext().GetCurrentFrameID();
		auto found = m_ActiveBuffers.find(returnBuffer->m_Buffer);
		if(found != m_ActiveBuffers.end())
		{
			m_PendingReleasingBuffers.emplace_back(std::make_tuple(found->first, found->second, frameId));
			m_ActiveBuffers.erase(found);
		}
		m_BufferObjectPool.Release(returnBuffer);
	}

	void CGlobalMemoryPool::ReleaseResourcesBeforeFrame(FrameType frame)
	{
		std::lock_guard<std::mutex> guard(m_Mutex);
		while((!m_PendingReleasingBuffers.empty())
			&& frame >= std::get<2>(m_PendingReleasingBuffers.front()))
		{
			auto frontBuffer = m_PendingReleasingBuffers.front();
			m_PendingReleasingBuffers.pop_front();
			vmaDestroyBuffer(m_BufferAllocator
				, std::get<0>(frontBuffer)
				, std::get<1>(frontBuffer));
		}
	}

	void CGlobalMemoryPool::Initialize()
	{
		VmaAllocatorCreateInfo vmaCreateInfo{};
		vmaCreateInfo.vulkanApiVersion = VULKAN_API_VERSION_IN_USE;
		vmaCreateInfo.physicalDevice = static_cast<VkPhysicalDevice>(GetPhysicalDevice());
		vmaCreateInfo.device = GetDevice();
		vmaCreateInfo.instance = GetInstance();
		vmaCreateAllocator(&vmaCreateInfo, &m_BufferAllocator);
	}

	void CGlobalMemoryPool::Release()
	{
		std::lock_guard<std::mutex> guard(m_Mutex);
		for(auto pending_release_buffer : m_PendingReleasingBuffers)
		{
			vmaDestroyBuffer(m_BufferAllocator
				, std::get<0>(pending_release_buffer)
				, std::get<1>(pending_release_buffer));
		}
		m_PendingReleasingBuffers.clear();
		for (auto pending_release_buffer : m_ActiveBuffers)
		{
			vmaDestroyBuffer(m_BufferAllocator
				, pending_release_buffer.first
				, pending_release_buffer.second);
		}
		m_ActiveBuffers.clear();
		vmaDestroyAllocator(m_BufferAllocator);
		m_BufferAllocator = nullptr;
	}

	CVulkanMemoryManager::CVulkanMemoryManager(CVulkanApplication& owner) : 
		BaseApplicationSubobject(owner)
		, m_GlobalMemoryPool(owner)
	{
	}

	std::shared_ptr<CVulkanBufferObject> CVulkanMemoryManager::AllocateBuffer(EMemoryType memoryType, EMemoryLifetime lifetime,
	                                                         size_t bufferSize, vk::BufferUsageFlags bufferUsage)
	{
		switch(lifetime)
		{
		case EMemoryLifetime::FrameBound:
			{
				uint32_t poolIndex = GetFrameCountContext().GetCurrentFrameBufferIndex();
				return m_FrameBoundPool[poolIndex].AllocateSharedBuffer(memoryType, bufferSize, bufferUsage);
			}
		case EMemoryLifetime::Persistent:
			return m_GlobalMemoryPool.AllocateSharedBuffer(memoryType, bufferSize, bufferUsage);
		}
		return nullptr;
	}

	std::shared_ptr<CVulkanBufferObject> CVulkanMemoryManager::AllocateFrameBoundTransferStagingBuffer(size_t bufferSize)
	{
		return AllocateBuffer(
			EMemoryType::CPU_Sequential_Access
			, EMemoryLifetime::FrameBound
			, bufferSize
			, vk::BufferUsageFlagBits::eTransferSrc);
	}

	void CVulkanMemoryManager::ReleaseCurrentFrameResource()
	{
		if (!GetFrameCountContext().AnyFrameFinished())
			return;
		TIndex const releasingPoolIndex = GetFrameCountContext().GetReleasedResourcePoolIndex();
		FrameType const releasingFrame = GetFrameCountContext().GetReleasedFrameID();
		m_GlobalMemoryPool.ReleaseResourcesBeforeFrame(releasingFrame);
		m_FrameBoundPool[releasingPoolIndex].ReleaseAllBuffers();
	}

	void CVulkanMemoryManager::Initialize()
	{
		m_GlobalMemoryPool.Initialize();
		m_FrameBoundPool.clear();
		//m_FrameBoundPool.reserve(FRAMEBOUND_RESOURCE_POOL_SWAP_COUNT_PER_CONTEXT);
		for(uint32_t i = 0; i < FRAMEBOUND_RESOURCE_POOL_SWAP_COUNT_PER_CONTEXT; ++i)
		{
			m_FrameBoundPool.emplace_back(i, GetVulkanApplication());
			m_FrameBoundPool[i].Initialize();
		}
	}

	void CVulkanMemoryManager::Release()
	{
		m_GlobalMemoryPool.Release();
		for(auto& itrPool : m_FrameBoundPool)
		{
			itrPool.Release();
		}
		m_FrameBoundPool.clear();
	}
	std::shared_ptr<CVulkanBufferObject> IVulkanBufferPool::AllocateSharedBuffer(
		EMemoryType memoryType
		, size_t bufferSize
		, vk::BufferUsageFlags bufferUsage)
	{
		return std::shared_ptr<CVulkanBufferObject>(AllocateBuffer(memoryType, bufferSize, bufferUsage), [this](CVulkanBufferObject* removingBuffer)
			{
				ReleaseBuffer(removingBuffer);
			});
	}
}
