#include <private/include/pch.h>
#include <private/include/RenderBackendSettings.h>
#include <private/include/CVulkanApplication.h>
#include <private/include/FrameCountContext.h>

namespace graphics_backend
{
	CSwapchainAndFrameCountContext::CSwapchainAndFrameCountContext(vk::SurfaceKHR targetSurface) : 
		m_TargetSurface(targetSurface)
	{
	}
	void CSwapchainAndFrameCountContext::Initialize_Internal(CVulkanApplication const* owningApplication)
	{
		assert(m_SwapchainWaitingFences.empty());
		m_CurrentFrameCount = 0;
		m_SwapchainWaitingFences.reserve(SWAPCHAIN_BUFFER_COUNT);
		for (uint32_t itrFenceId = 0; itrFenceId < SWAPCHAIN_BUFFER_COUNT; ++itrFenceId)
		{
			vk::FenceCreateInfo fenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);
			vk::Fence && newFence = m_OwningApplication->GetDevice().createFence(fenceCreateInfo);
			m_SwapchainWaitingFences.push_back(newFence);
		}

		//vk::SwapchainCreateInfoKHR swapchainCreateInfo(vk::SwapchainCreateFlagBitsKHR::eDeferredMemoryAllocationEXT, m_TargetSurface
		//	, SWAPCHAIN_BUFFER_COUNT, vk::Format::eB10G11R11UfloatPack32, vk::ColorSpaceKHR::eAdobergbLinearEXT, )
		//m_OwningApplication->GetDevice().createSwapchainKHR()
	}

	void CSwapchainAndFrameCountContext::Release_Internal()
	{
		for (auto& fence : m_SwapchainWaitingFences)
		{
			m_OwningApplication->GetDevice().destroyFence(fence);
		}
		m_SwapchainWaitingFences.clear();
	}
}

