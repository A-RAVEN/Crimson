#pragma once
#include <include/Generals.h>
#include <headers/VulkanHeader.h>
#include <headers/VulkanGPUDevice.h>
#include <headers/VulkanFramebuffer.h>
#include <headers/VulkanRenderPass.h>

namespace Crimson
{
	class VulkanRenderPassInstance : public RenderPassInstance
	{
	public:
		friend class VulkanGPUDevice;
		friend class VulkanGPUDeviceThread;
		friend class VulkanGraphicsCommandBuffer;
		friend class RenderPassInstanceGraphicsCommandBufferInfo;
		friend class VulkanExecutionCommandBuffer;

		VulkanRenderPassInstance();
		~VulkanRenderPassInstance();
		virtual void Call() override;
		virtual void Dispose() override;
		void InitRenderPassInstance(VulkanGPUDevice* device, VulkanRenderPass* render_pass, VulkanFramebuffer* framebuffer, uint64_t unique_id);
	private:
		VulkanGPUDevice*				p_OwningDevice;
		VkFramebuffer					m_Framebuffer;
		VulkanExecutionCommandBuffer*	p_OwningExecutionCommandBuffer;
		uint64_t						m_InstanceUniqueId;
	};
}