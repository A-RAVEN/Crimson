#pragma once
#include  <vulkan/vulkan.hpp>
#include <private/include/RenderBackendSettings.h>
#include <private/include/VulkanApplicationSubobjectBase.h>
#include <VulkanMemoryAllocator/include/vk_mem_alloc.h>
#include <private/include/CVulkanBufferObject.h>
#include <deque>

namespace graphics_backend
{
#pragma region Forward Declaration
	class CVulkanApplication;
#pragma endregion

	class CVulkanFrameBoundCommandBufferPool : public ApplicationSubobjectBase
	{
	public:
		vk::CommandBuffer AllocateCommandBuffer();
		vk::CommandBuffer AllocateSecondaryCommandBuffer();
		void ResetCommandBufferPool();
		void CollectCommandBufferList(std::vector<vk::CommandBuffer>& inoutCommandBufferList);
		uint32_t GetCommandFrame() const { return m_BoundingFrameID; }
	private:
		// 通过 ApplicationSubobjectBase 继承
		virtual void Initialize_Internal(CVulkanApplication const* owningApplication) override;
		virtual void Release_Internal() override;
	private:
		class CommandBufferList
		{
		public:
			size_t m_AvailableCommandBufferIndex = 0;
			std::vector<vk::CommandBuffer> m_CommandBufferList;
			vk::CommandBuffer AllocCommandBuffer(CVulkanFrameBoundCommandBufferPool& owner, bool secondary);
			void CollectCommandBufferList(std::vector<vk::CommandBuffer>& inoutCommandBufferList);
			void ResetBufferList();
			void ClearBufferList();
		};
		vk::CommandPool m_CommandPool = nullptr;
		CommandBufferList m_CommandBufferList;
		CommandBufferList m_SecondaryCommandBufferList;
		uint32_t m_BoundingFrameID = 0;
	};

	class CVulkanThreadContext : public ApplicationSubobjectBase
	{
	public:
		CVulkanThreadContext(uint32_t threadId);

		CVulkanFrameBoundCommandBufferPool& GetCurrentFramePool();
		void CollectSubmittingCommandBuffers(std::vector<vk::CommandBuffer>& inoutCommandBufferList);
		std::shared_ptr<CVulkanBufferObject> AllocBufferObject(bool gpuBuffer, uint32_t bufferSize, vk::BufferUsageFlags bufferUsage);
		void ReleaseBufferObject(CVulkanBufferObject* bufferObject);
	private:
		// 通过 ApplicationSubobjectBase 继承
		virtual void Initialize_Internal(CVulkanApplication const* owningApplication) override;
		virtual void Release_Internal() override;
	private:
		uint32_t m_ThreadID;
		std::vector<CVulkanFrameBoundCommandBufferPool> m_FrameBoundCommandBufferPools;
		VmaAllocator m_ThreadGPUAllocator = nullptr;
		std::set<VmaAllocation> m_HoldingAllocations;

		std::deque<std::tuple<vk::Buffer, VmaAllocation, uint32_t>> m_PendingRemovalBuffers;
	};
}
