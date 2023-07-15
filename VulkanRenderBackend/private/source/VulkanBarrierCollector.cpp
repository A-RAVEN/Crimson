#include <private/include/pch.h>
#include <private/include/VulkanBarrierCollector.h>

namespace graphics_backend
{

	static std::array<ResourceUsageVulkanInfo, 4> s_ResourceUsageToVulkanInfo = {
		ResourceUsageVulkanInfo{
			vk::PipelineStageFlagBits::eTransfer,
			vk::AccessFlagBits::eTransferRead,
			vk::ImageLayout::eTransferSrcOptimal,
		},
		ResourceUsageVulkanInfo{
			vk::PipelineStageFlagBits::eTransfer,
			vk::AccessFlagBits::eTransferWrite,
			vk::ImageLayout::eTransferDstOptimal,
		},
		ResourceUsageVulkanInfo{
			vk::PipelineStageFlagBits::eVertexShader,
			vk::AccessFlagBits::eShaderRead,
			vk::ImageLayout::eShaderReadOnlyOptimal,
		},
		ResourceUsageVulkanInfo{
			vk::PipelineStageFlagBits::eVertexShader,
			vk::AccessFlagBits::eShaderWrite,
			vk::ImageLayout::eGeneral,
		},
		ResourceUsageVulkanInfo{
			vk::PipelineStageFlagBits::eFragmentShader,
			vk::AccessFlagBits::eShaderRead,
			vk::ImageLayout::eShaderReadOnlyOptimal,
		},
		ResourceUsageVulkanInfo{
			vk::PipelineStageFlagBits::eFragmentShader,
			vk::AccessFlagBits::eShaderWrite,
			vk::ImageLayout::eGeneral,
		},
		ResourceUsageVulkanInfo{
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::AccessFlagBits::eColorAttachmentWrite,
			vk::ImageLayout::eAttachmentOptimal,
		},
	};

}