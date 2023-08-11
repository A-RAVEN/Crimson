#pragma once
#include  <vulkan/vulkan.hpp>
#include <private/include/RenderBackendSettings.h>
#include <private/include/VulkanApplicationSubobjectBase.h>
#include <VulkanMemoryAllocator/include/vk_mem_alloc.h>
#include <deque>

#include "CVulkanBufferObject.h"
#include <map>

namespace graphics_backend
{
	enum class EMemoryType
	{
		//gpu可见
		GPU,
		//cpu可见，随机读写
		CPU_Random_Access,
		//cpu可见，顺序读写
		CPU_Sequential_Access,
	};

	enum class EMemoryLifetime
	{
		Persistent,
		FrameBound,
	};

	class CFrameBoundMemoryPool : public BaseApplicationSubobject
	{
	public:
		CFrameBoundMemoryPool(uint32_t pool_id, CVulkanApplication& owner);
		CFrameBoundMemoryPool(CFrameBoundMemoryPool&& other) noexcept;
		CVulkanBufferObject AllocateFrameBoundBuffer(EMemoryType memoryType, size_t bufferSize, vk::BufferUsageFlags bufferUsage);
		void ReleaseBuffer(CVulkanBufferObject& returnBuffer);
		void ReleaseAllBuffers();
		void Initialize() override;
		void Release() override;
	private:
		std::mutex m_Mutex;
		VmaAllocator m_FrameBoundAllocator = nullptr;
		std::deque<std::tuple<vk::Buffer, VmaAllocation>> m_ActiveBuffers;
		uint32_t m_PoolId;
	};

	class CGlobalMemoryPool : public BaseApplicationSubobject
	{
	public:
		CGlobalMemoryPool(CVulkanApplication& owner);
		CVulkanBufferObject AllocatePersistantBuffer(EMemoryType memoryType, size_t bufferSize, vk::BufferUsageFlags bufferUsage);
		void ReleaseBuffer(CVulkanBufferObject& returnBuffer);
		void ReleaseResourcesBeforeFrame(FrameType frame);
		void Initialize() override;
		void Release() override;
	private:
		VmaAllocator m_BufferAllocator = nullptr;
		std::mutex m_Mutex;
		std::deque<std::tuple<vk::Buffer, VmaAllocation, FrameType>> m_PendingReleasingBuffers;
		std::map<vk::Buffer, VmaAllocation> m_ActiveBuffers;
	};

	class CVulkanMemoryManager : public BaseApplicationSubobject
	{
	public:
		CVulkanMemoryManager(CVulkanApplication& owner);
		CVulkanBufferObject AllocateBuffer(EMemoryType memoryType, EMemoryLifetime lifetime, size_t bufferSize, vk::BufferUsageFlags bufferUsage);
		void ReleaseBuffer(CVulkanBufferObject& returnBuffer);
		void ReleaseCurrentFrameResource();
	protected:
		void Initialize() override;
		void Release() override;
	private:
		CGlobalMemoryPool m_GlobalMemoryPool;
		std::vector<CFrameBoundMemoryPool> m_FrameBoundPool;
	};
}
