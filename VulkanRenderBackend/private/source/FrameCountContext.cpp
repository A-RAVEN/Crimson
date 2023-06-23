#include <private/include/pch.h>
#include <private/include/RenderBackendSettings.h>
#include <private/include/CVulkanApplication.h>
#include <private/include/FrameCountContext.h>

namespace graphics_backend
{
	void CFrameCountContext::WaitingForCurrentFrame()
	{
		uint32_t currentIndex = GetCurrentFrameBufferIndex();
		uint32_t waitingFrame = m_FenceSubmitFrameIDs[currentIndex];
		std::vector<vk::Fence> fences = {
			m_SubmitFrameFences[currentIndex]
		};
		GetVulkanApplication()->GetDevice().waitForFences(fences
			, true
			, std::numeric_limits<uint64_t>::max());
		m_LastFinshedFrameID = m_FenceSubmitFrameIDs[currentIndex];
		m_FenceSubmitFrameIDs[currentIndex] = m_CurrentFrameID;
	}
	void CFrameCountContext::SubmitCurrentFrameGraphics(std::vector<vk::CommandBuffer> const& commandbufferList)
	{
		uint32_t currentIndex = GetCurrentFrameBufferIndex();
		vk::Fence currentFrameFence = m_SubmitFrameFences[currentIndex];
		vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		vk::SubmitInfo submitInfo({}, waitDestinationStageMask, commandbufferList);
		m_GraphicsQueue.submit(submitInfo, currentFrameFence);
		++m_CurrentFrameID;
	}
	void CFrameCountContext::SubmitCurrentFrameCompute(std::vector<vk::CommandBuffer> const& commandbufferList)
	{

	}
	void CFrameCountContext::SubmitCurrentFrameTransfer(std::vector<vk::CommandBuffer> const& commandbufferList)
	{
	}
	void CFrameCountContext::InitializeSubmitQueues(vk::Queue const& generalQueue, vk::Queue const& computeQueue, vk::Queue const& transferQueue)
	{
		m_GraphicsQueue = generalQueue;
		m_ComputeQueue = computeQueue;
		m_TransferQueue = transferQueue;
	}
	void CFrameCountContext::Initialize_Internal(CVulkanApplication const* owningApplication)
	{

		assert(m_SubmitFrameFences.empty());
		m_CurrentFrameID = 0;
		m_SubmitFrameFences.reserve(SWAPCHAIN_BUFFER_COUNT);
		for (uint32_t itrFenceId = 0; itrFenceId < SWAPCHAIN_BUFFER_COUNT; ++itrFenceId)
		{
			vk::FenceCreateInfo fenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);
			vk::Fence && newFence = m_OwningApplication->GetDevice().createFence(fenceCreateInfo);
			m_SubmitFrameFences.push_back(newFence);
		}
		m_FenceSubmitFrameIDs.resize(SWAPCHAIN_BUFFER_COUNT);
		std::fill(m_FenceSubmitFrameIDs.begin(), m_FenceSubmitFrameIDs.end(), std::numeric_limits<uint32_t>::max());
	}

	void CFrameCountContext::Release_Internal()
	{
		for (auto& fence : m_SubmitFrameFences)
		{
			m_OwningApplication->GetDevice().destroyFence(fence);
		}
		m_SubmitFrameFences.clear();
		m_FenceSubmitFrameIDs.clear();
		m_GraphicsQueue = nullptr;
		m_ComputeQueue = nullptr;
		m_TransferQueue = nullptr;
	}
	void CFrameCountContext::Initialize_Submit_Queues()
	{

	}
}

