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
		uint32_t GetCurrentFrameBufferIndex() const {
			return m_CurrentFrameID % FRAMEBOUND_COMMANDPOOL_SWAP_COUNT_PER_CONTEXT;
		}
		uint32_t GetCurrentFrameID() const {
			return m_CurrentFrameID;
		}
		uint32_t GetReleasedFrameID() const {
			return m_LastFinshedFrameID;
		}
	private:

		// 通过 ApplicationSubobjectBase 继承
		virtual void Initialize_Internal(CVulkanApplication const* owningApplication) override;
		virtual void Release_Internal() override;
		void Initialize_Submit_Queues();
	private:
		std::atomic<uint32_t> m_CurrentFrameID = 0;
		uint32_t m_LastFinshedFrameID = 0;
		std::vector<vk::Fence> m_SubmitFrameFences;
		std::vector<uint32_t> m_FenceSubmitFrameIDs;
		vk::Queue m_GraphicsQueue = nullptr;
		vk::Queue m_ComputeQueue = nullptr;
		vk::Queue m_TransferQueue = nullptr;
	};
}