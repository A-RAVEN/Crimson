#pragma once
#include <private/include/VulkanApplicationSubobjectBase.h>
namespace graphics_backend
{
	class CSwapchainAndFrameCountContext : public ApplicationSubobjectBase
	{
	private:
		CSwapchainAndFrameCountContext(vk::SurfaceKHR targetSurface);
		// Í¨¹ý ApplicationSubobjectBase ¼Ì³Ð
		virtual void Initialize_Internal(CVulkanApplication const* owningApplication) override;
		virtual void Release_Internal() override;
	private:
		vk::SurfaceKHR m_TargetSurface;
		uint32_t m_CurrentFrameCount = 0;
		std::vector<vk::Fence> m_SwapchainWaitingFences;
		vk::SwapchainKHR m_Swapchain = nullptr;
	};
}