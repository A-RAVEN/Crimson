#pragma once
#include <private/include/VulkanApplicationSubobjectBase.h>
namespace graphics_backend
{
	class CFrameCountContext : public ApplicationSubobjectBase
	{
	public:
		void WaitingForCurrentFrame();
		void EndCurrentFrame();
		void SubmitGraphics(std::vector<vk::CommandBuffer> const& commandbufferList
			, vk::ArrayProxyNoTemporaries<const vk::Semaphore> waitSemaphores = {}
			, vk::ArrayProxyNoTemporaries<const vk::Semaphore> signalSemaphores = {});

		void FinalizeCurrentFrameGraphics(std::vector<vk::CommandBuffer> const& commandbufferList
			, vk::ArrayProxyNoTemporaries<const vk::Semaphore> waitSemaphores = {}
			, vk::ArrayProxyNoTemporaries<const vk::PipelineStageFlags> waitStages = {}
			, vk::ArrayProxyNoTemporaries<const vk::Semaphore> signalSemaphores = {});

		void SubmitCurrentFrameCompute(std::vector<vk::CommandBuffer> const& commandbufferList);
		void SubmitCurrentFrameTransfer(std::vector<vk::CommandBuffer> const& commandbufferList);
		void InitializeSubmitQueues(std::pair<uint32_t, uint32_t> const& generalQueue
			, std::pair<uint32_t, uint32_t> const& computeQueue
			, std::pair<uint32_t, uint32_t> const& transferQueue);
		void InitializeDefaultQueues(std::vector<vk::Queue> defaultQueues);
		TIndex FindPresentQueueFamily(vk::SurfaceKHR surface) const;
		std::pair<uint32_t, vk::Queue> FindPresentQueue(vk::SurfaceKHR surface) const;
		TIndex GetCurrentFrameBufferIndex() const {
			return m_CurrentFrameID % FRAMEBOUND_RESOURCE_POOL_SWAP_COUNT_PER_CONTEXT;
		}
		TIndex GetReleasedResourcePoolIndex() const
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

		std::pair<uint32_t, uint32_t> const& GetGraphicsQueueRef() const
		{
			return m_GraphicsQueueReference;
		}

		uint32_t GetGraphicsQueueFamily() const
		{
			return m_GraphicsQueueReference.first;
		}

		uint32_t GetTransferQueueFamily() const
		{
			return m_TransferQueueReference.first;
		}

		uint32_t GetComputeQueueFamily() const
		{
			return m_ComputeQueueReference.first;
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
		std::pair<uint32_t, uint32_t> m_GraphicsQueueReference;
		vk::Queue m_GraphicsQueue = nullptr;
		std::pair<uint32_t, uint32_t> m_ComputeQueueReference;
		vk::Queue m_ComputeQueue = nullptr;
		std::pair<uint32_t, uint32_t> m_TransferQueueReference;
		vk::Queue m_TransferQueue = nullptr;

		std::vector<vk::Queue> m_QueueFamilyDefaultQueues;
	};
}