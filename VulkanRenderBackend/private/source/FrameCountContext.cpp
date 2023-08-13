#include <private/include/pch.h>
#include <private/include/RenderBackendSettings.h>
#include <private/include/CVulkanApplication.h>
#include <private/include/FrameCountContext.h>

namespace graphics_backend
{
	void CFrameCountContext::WaitingForCurrentFrame()
	{
		TIndex currentIndex = GetCurrentFrameBufferIndex();
		FrameType waitingFrame = m_FenceSubmitFrameIDs[currentIndex];
		std::atomic_thread_fence(std::memory_order_acquire);
		std::vector<vk::Fence> fences = {
			m_SubmitFrameFences[currentIndex]
		};
		GetVulkanApplication()->GetDevice().waitForFences(fences
			, true
			, std::numeric_limits<uint64_t>::max());
		std::atomic_thread_fence(std::memory_order_release);
		m_LastFinshedFrameID = waitingFrame;
		m_FenceSubmitFrameIDs[currentIndex] = m_CurrentFrameID;
	}

	void CFrameCountContext::EndCurrentFrame()
	{
		std::atomic_thread_fence(std::memory_order_release);
		++m_CurrentFrameID;
	}

	void CFrameCountContext::SubmitGraphics(std::vector<vk::CommandBuffer> const& commandbufferList,
		vk::ArrayProxyNoTemporaries<const vk::Semaphore> waitSemaphores,
		vk::ArrayProxyNoTemporaries<const vk::Semaphore> signalSemaphores)
	{
		if (commandbufferList.empty() && waitSemaphores.empty() && signalSemaphores.empty())
			return;
		vk::SubmitInfo const submitInfo(waitSemaphores, {}, commandbufferList, signalSemaphores);
		m_GraphicsQueue.submit(submitInfo);
	}

	void CFrameCountContext::FinalizeCurrentFrameGraphics(
		std::vector<vk::CommandBuffer> const& commandbufferList
	    , vk::ArrayProxyNoTemporaries<const vk::Semaphore> waitSemaphores
		, vk::ArrayProxyNoTemporaries<const vk::PipelineStageFlags> waitStages
		, vk::ArrayProxyNoTemporaries<const vk::Semaphore> signalSemaphores)
	{
		uint32_t currentIndex = GetCurrentFrameBufferIndex();
		vk::Fence currentFrameFence = m_SubmitFrameFences[currentIndex];
		std::vector<vk::Fence> fences = {
			m_SubmitFrameFences[currentIndex]
		};
		GetDevice().resetFences(fences);
		vk::SubmitInfo submitInfo(waitSemaphores, waitStages, commandbufferList, signalSemaphores);
		m_GraphicsQueue.submit(submitInfo, currentFrameFence);
	}
	void CFrameCountContext::SubmitCurrentFrameCompute(std::vector<vk::CommandBuffer> const& commandbufferList)
	{

	}
	void CFrameCountContext::SubmitCurrentFrameTransfer(std::vector<vk::CommandBuffer> const& commandbufferList)
	{
	}
	void CFrameCountContext::InitializeSubmitQueues(
		std::pair<uint32_t, uint32_t> const& generalQueue
		, std::pair<uint32_t, uint32_t> const& computeQueue
		, std::pair<uint32_t, uint32_t> const& transferQueue)
	{
		m_GraphicsQueueReference = generalQueue;
		m_ComputeQueueReference = computeQueue;
		m_TransferQueueReference = transferQueue;
		m_GraphicsQueue = GetDevice().getQueue(m_GraphicsQueueReference.first, m_GraphicsQueueReference.second);
		m_ComputeQueue = GetDevice().getQueue(m_ComputeQueueReference.first, m_ComputeQueueReference.second);
		m_TransferQueue = GetDevice().getQueue(m_TransferQueueReference.first, m_TransferQueueReference.second);
	}

	void CFrameCountContext::InitializeDefaultQueues(std::vector<vk::Queue> defaultQueues)
	{
		m_QueueFamilyDefaultQueues = defaultQueues;
	}

	uint32_t CFrameCountContext::FindPresentQueueFamily(vk::SurfaceKHR surface) const
	{
		for(uint32_t familyId = 0; familyId < m_QueueFamilyDefaultQueues.size(); ++familyId)
		{
			if(GetPhysicalDevice().getSurfaceSupportKHR(familyId, surface))
			{
				return familyId;
			}
		}
		return std::numeric_limits<uint32_t>::max();
	}

	std::pair<uint32_t, vk::Queue> CFrameCountContext::FindPresentQueue(vk::SurfaceKHR surface) const
	{
		for (uint32_t familyId = 0; familyId < m_QueueFamilyDefaultQueues.size(); ++familyId)
		{
			if (GetPhysicalDevice().getSurfaceSupportKHR(familyId, surface))
			{
				return std::make_pair(familyId, m_QueueFamilyDefaultQueues[familyId]);
			}
		}
		assert(false);
		return std::pair<uint32_t, vk::Queue>(INVALID_INDEX, nullptr);
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
		std::fill(m_FenceSubmitFrameIDs.begin(), m_FenceSubmitFrameIDs.end(), INVALID_FRAMEID);
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
}

