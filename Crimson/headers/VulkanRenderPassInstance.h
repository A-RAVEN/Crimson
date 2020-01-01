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
		VulkanRenderPassInstance();
		~VulkanRenderPassInstance();
		void InitRenderPassInstance(VulkanGPUDevice* device, VulkanRenderPass* render_pass, VulkanFramebuffer* framebuffer);
	private:
		VulkanGPUDevice* p_OwningDevice;
		VkFramebuffer	m_Framebuffer;
	};
}