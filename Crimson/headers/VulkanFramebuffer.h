#pragma once
#include <include/Generals.h>
#include <headers/VulkanHeader.h>

namespace Crimson
{
	class VulkanFramebuffer : public Framebuffer
	{
	public:
		friend class VulkanRenderPassInstance;

		VulkanFramebuffer();
		~VulkanFramebuffer();
		uint32_t GetWidth();
		uint32_t GetHeight();
		uint32_t GetLayers();

		void ImageBarriers(VkCommandBuffer cmd_buffer, uint32_t cmd_queue_family);
		std::vector< VkImageMemoryBarrier> m_ImageMemoryBarriers;
	};
}