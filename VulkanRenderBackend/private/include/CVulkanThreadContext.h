#pragma once
#include  <vulkan/vulkan.hpp>
#include <private/include/RenderBackendSettings.h>
#include <private/include/VulkanApplicationSubobjectBase.h>

namespace graphics_backend
{
#pragma region Forward Declaration
	class CVulkanApplication;
#pragma endregion

	class CVulkanFrameBoundCommandBufferPool : public ApplicationSubobjectBase
	{
	public:
		vk::CommandBuffer AllocateCommandBuffer();
		void ResetCommandBufferPool();
	private:
		// 通过 ApplicationSubobjectBase 继承
		virtual void Initialize_Internal(CVulkanApplication const* owningApplication) override;
		virtual void Release_Internal() override;
	private:
		vk::CommandPool m_CommandPool = nullptr;
		std::vector<vk::CommandBuffer> m_CommandBufferList;
		uint32_t m_BoundingFrameID = 0;
	};

	class CVulkanThreadContext : public ApplicationSubobjectBase
	{
	public:
		CVulkanThreadContext(CVulkanApplication const* owningApplication);
		CVulkanFrameBoundCommandBufferPool& GetCurrentFramePool();

	private:
		// 通过 ApplicationSubobjectBase 继承
		virtual void Initialize_Internal(CVulkanApplication const* owningApplication) override;
		virtual void Release_Internal() override;
	private:
		std::vector<CVulkanFrameBoundCommandBufferPool> m_FrameBoundCommandBufferPools;
	};
}
