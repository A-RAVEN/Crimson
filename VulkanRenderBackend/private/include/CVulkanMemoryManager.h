#pragma once
#include  <vulkan/vulkan.hpp>
#include <private/include/RenderBackendSettings.h>
#include <private/include/VulkanApplicationSubobjectBase.h>
#include <VulkanMemoryAllocator/include/vk_mem_alloc.h>
#include <deque>
#include <map>
#include "CVulkanBufferObject.h"
#include "Containers.h"
#include "VulkanImageObject.h"
#include <RenderInterface/header/GPUTexture.h>

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

	class IVulkanBufferPool
	{
	public:
		virtual CVulkanBufferObject* AllocateBuffer(EMemoryType memoryType, size_t bufferSize, vk::BufferUsageFlags bufferUsage) = 0;
		virtual void ReleaseBuffer(CVulkanBufferObject* returnBuffer) = 0;
		std::shared_ptr<CVulkanBufferObject> AllocateSharedBuffer(EMemoryType memoryType, size_t bufferSize, vk::BufferUsageFlags bufferUsage);
	};

	class IVulkanImagePool
	{
	public:
		virtual VulkanImageObject AllocateImage(GPUTextureDescriptor const& textureDescriptor) = 0;
		virtual void ReleaseImage(VulkanImageObject& releasingImage) = 0;
	};

	class CFrameBoundMemoryPool : public BaseApplicationSubobject, public IVulkanBufferPool
	{
	public:
		CFrameBoundMemoryPool(uint32_t pool_id, CVulkanApplication& owner);
		//CFrameBoundMemoryPool(CFrameBoundMemoryPool&& other) noexcept;
		virtual CVulkanBufferObject* AllocateBuffer(EMemoryType memoryType, size_t bufferSize, vk::BufferUsageFlags bufferUsage) override;
		virtual void ReleaseBuffer(CVulkanBufferObject* returnBuffer) override;
		void ReleaseAllBuffers();
		void Initialize();
		void Release() override;
	private:
		std::mutex m_Mutex;
		VmaAllocator m_BufferAllocator = nullptr;
		std::deque<std::tuple<vk::Buffer, VmaAllocation>> m_ActiveBuffers;
		uint32_t m_PoolId;
		TVulkanApplicationPool<CVulkanBufferObject> m_BufferObjectPool;
	};


	class CGlobalMemoryPool : public BaseApplicationSubobject, public IVulkanBufferPool
	{
	public:
		CGlobalMemoryPool(CVulkanApplication& owner);
		CVulkanBufferObject* AllocateBuffer(EMemoryType memoryType, size_t bufferSize, vk::BufferUsageFlags bufferUsage);
		void ReleaseBuffer(CVulkanBufferObject* returnBuffer);
		void ReleaseResourcesBeforeFrame(FrameType frame);
		void Initialize();
		void Release() override;

		VulkanImageObject AllocateImage(GPUTextureDescriptor const& textureDescriptor, EMemoryType memoryType);
	private:
		VmaAllocator m_GlobalAllocator = nullptr;
		std::mutex m_Mutex;
		std::deque<std::tuple<vk::Buffer, VmaAllocation, FrameType>> m_PendingReleasingBuffers;
		std::map<vk::Buffer, VmaAllocation> m_ActiveBuffers;
		TVulkanApplicationPool<CVulkanBufferObject> m_BufferObjectPool;

		void ScheduleReleaseImage(VulkanImageObject_Internal& releasingImage);
		void ReleaseImage_Internal(std::deque<VulkanImageObject_Internal> const& releasingObjects);
		TFrameboundReleaser<VulkanImageObject_Internal> m_ImageFrameboundReleaser;
	};

	class CVulkanMemoryManager : public BaseApplicationSubobject
	{
	public:
		CVulkanMemoryManager(CVulkanApplication& owner);
		std::shared_ptr<CVulkanBufferObject> AllocateBuffer(EMemoryType memoryType, EMemoryLifetime lifetime, size_t bufferSize, vk::BufferUsageFlags bufferUsage);
		std::shared_ptr<CVulkanBufferObject> AllocateFrameBoundTransferStagingBuffer(size_t bufferSize);
		VulkanImageObject AllocateImage(GPUTextureDescriptor const& textureDescriptor, EMemoryType memoryType, EMemoryLifetime lifetime);
		void ReleaseCurrentFrameResource();
		void Initialize();
		void Release() override;
	private:
		CGlobalMemoryPool m_GlobalMemoryPool;
		std::deque<CFrameBoundMemoryPool> m_FrameBoundPool;
	};
}
