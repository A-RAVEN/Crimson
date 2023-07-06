#pragma once
#include <private/include/VulkanApplicationSubobjectBase.h>
namespace graphics_backend
{
	class CFrameCountContext : public ApplicationSubobjectBase
	{
	public:
		void WaitingForCurrentFrame();
		void SubmitCurrentFrameGraphics(std::vector<vk::CommandBuffer> const& commandbufferList);
		void SubmitCurrentFrameCompute(std::vector<vk::CommandBuffer> const& commandbufferList);
		void SubmitCurrentFrameTransfer(std::vector<vk::CommandBuffer> const& commandbufferList);
		void InitializeSubmitQueues(vk::Queue const& generalQueue
			, vk::Queue const& computeQueue
			, vk::Queue const& transferQueue);
		void InitializeDefaultQueues(std::vector<vk::Queue> defaultQueues);
		uint32_t FindPresentQueueFamily(vk::SurfaceKHR surface) const;
		vk::Queue FindPresentQueue(vk::SurfaceKHR surface) const;
		uint32_t GetCurrentFrameBufferIndex() const {
			return m_CurrentFrameID % FRAMEBOUND_RESOURCE_POOL_SWAP_COUNT_PER_CONTEXT;
		}
		uint32_t GetReleasedResourcePoolIndex() const
		{
			return m_LastFinshedFrameID % FRAMEBOUND_RESOURCE_POOL_SWAP_COUNT_PER_CONTEXT;
		}
		FrameType GetCurrentFrameID() const {
			return m_CurrentFrameID;
		}
		FrameType GetReleasedFrameID() const {
			return m_LastFinshedFrameID;
		}
		bool AnyFrameFinished() const
		{
			return m_LastFinshedFrameID != INVALID_FRAMEID;
		}
	private:
		// 通过 ApplicationSubobjectBase 继承
		virtual void Initialize_Internal(CVulkanApplication const* owningApplication) override;
		virtual void Release_Internal() override;
	private:
		std::atomic<FrameType> m_CurrentFrameID {0};
		FrameType m_LastFinshedFrameID = INVALID_FRAMEID;
		std::vector<vk::Fence> m_SubmitFrameFences;
		std::vector<FrameType> m_FenceSubmitFrameIDs;
		vk::Queue m_GraphicsQueue = nullptr;
		vk::Queue m_ComputeQueue = nullptr;
		vk::Queue m_TransferQueue = nullptr;

		std::vector<vk::Queue> m_QueueFamilyDefaultQueues;
	};
}