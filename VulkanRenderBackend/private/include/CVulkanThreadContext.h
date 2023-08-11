#pragma once
#include  <vulkan/vulkan.hpp>
#include <private/include/RenderBackendSettings.h>
#include <private/include/VulkanApplicationSubobjectBase.h>
#include <VulkanMemoryAllocator/include/vk_mem_alloc.h>
#include <private/include/CVulkanBufferObject.h>
#include <deque>
#include <mutex>

namespace graphics_backend
{
#pragma region Forward Declaration
	class CVulkanApplication;
#pragma endregion

	template<typename T>
	class Internal_InterlockedQueue
	{
	public:
		template<typename TContainer>
		void Initialize(TContainer const& initializer)
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			m_Queue.clear();
			m_Queue.resize(initializer.size());
			std::copy(initializer.begin(), initializer.end(), m_Queue.begin());
			m_Conditional.notify_one();
		}

		void Enqueue(T& newVar)
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			m_Queue.push_back(newVar);
			m_Conditional.notify_one();
		}
		T TryGetFront()
		{
			T result;
			{
				std::unique_lock<std::mutex> lock(m_Mutex);
				if (m_Queue.empty())
				{
					m_Conditional.wait(lock);
				}
				result = m_Queue.front();
				m_Queue.pop_front();
			}
			return result;
		}
	private:
		std::mutex m_Mutex;
		std::condition_variable m_Conditional;
		std::deque<T> m_Queue;
	};

	class CVulkanFrameBoundCommandBufferPool : public ApplicationSubobjectBase
	{
	public:
		vk::CommandBuffer AllocateOnetimeCommandBuffer();
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
		CVulkanFrameBoundCommandBufferPool& GetPoolByFrame(uint32_t poolID);
		void CollectSubmittingCommandBuffers(std::vector<vk::CommandBuffer>& inoutCommandBufferList);
		uint32_t GetThreadID() const { return m_ThreadID; }
		void DoReleaseResourceBeforeFrame(uint32_t releasingFrame);
	private:
		// 通过 ApplicationSubobjectBase 继承
		virtual void Initialize_Internal(CVulkanApplication const* owningApplication) override;
		virtual void Release_Internal() override;
	private:
		uint32_t m_ThreadID;
		std::vector<CVulkanFrameBoundCommandBufferPool> m_FrameBoundCommandBufferPools;
	};
}
