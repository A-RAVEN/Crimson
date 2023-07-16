#include <private/include/pch.h>
#include <private/include/VulkanBarrierCollector.h>

namespace graphics_backend
{

	const ResourceUsageVulkanInfo s_NoneUsageVulkanInfo{
		vk::PipelineStageFlagBits::eTopOfPipe,
		vk::AccessFlagBits::eNone,
		vk::ImageLayout::eUndefined,
	};

	const static ResourceUsageVulkanInfo s_ResourceUsageToVulkanInfo[] = {
		//eTransferSource
		ResourceUsageVulkanInfo{
			vk::PipelineStageFlagBits::eTransfer,
			vk::AccessFlagBits::eTransferRead,
			vk::ImageLayout::eTransferSrcOptimal,
		},
		//eTransferDest
		ResourceUsageVulkanInfo{
			vk::PipelineStageFlagBits::eTransfer,
			vk::AccessFlagBits::eTransferWrite,
			vk::ImageLayout::eTransferDstOptimal,
		},
		//eVertexRead
		ResourceUsageVulkanInfo{
			vk::PipelineStageFlagBits::eVertexShader,
			vk::AccessFlagBits::eShaderRead,
			vk::ImageLayout::eShaderReadOnlyOptimal,
		},
		//eVertexWrite
		ResourceUsageVulkanInfo{
			vk::PipelineStageFlagBits::eVertexShader,
			vk::AccessFlagBits::eShaderWrite,
			vk::ImageLayout::eGeneral,
		},
		//eFragmentRead
		ResourceUsageVulkanInfo{
			vk::PipelineStageFlagBits::eFragmentShader,
			vk::AccessFlagBits::eShaderRead,
			vk::ImageLayout::eShaderReadOnlyOptimal,
		},
		//eFragmentWrite
		ResourceUsageVulkanInfo{
			vk::PipelineStageFlagBits::eFragmentShader,
			vk::AccessFlagBits::eShaderWrite,
			vk::ImageLayout::eGeneral,
		},
		//eComputeRead
		ResourceUsageVulkanInfo{
			vk::PipelineStageFlagBits::eComputeShader,
			vk::AccessFlagBits::eShaderRead,
			vk::ImageLayout::eShaderReadOnlyOptimal,
		},
		//eComputeWrite
		ResourceUsageVulkanInfo{
			vk::PipelineStageFlagBits::eComputeShader,
			vk::AccessFlagBits::eShaderWrite,
			vk::ImageLayout::eGeneral,
		},
		//eColorAttachmentOutput
		ResourceUsageVulkanInfo{
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::AccessFlagBits::eColorAttachmentWrite,
			vk::ImageLayout::eAttachmentOptimal,
		},
		//ePresent
		ResourceUsageVulkanInfo{
			vk::PipelineStageFlagBits::eBottomOfPipe,
			vk::AccessFlagBits::eNone,
			vk::ImageLayout::ePresentSrcKHR,
		},
	};

	ResourceUsageVulkanInfo Combination(ResourceUsageVulkanInfo const& infoA
		, ResourceUsageVulkanInfo const& infoB)
	{
		ResourceUsageVulkanInfo result{};
		result.m_UsageStageMask = infoA.m_UsageStageMask | infoB.m_UsageStageMask;
		result.m_UsageAccessFlags = infoA.m_UsageAccessFlags | infoB.m_UsageAccessFlags;
		result.m_UsageImageLayout = (infoA.m_UsageImageLayout == infoB.m_UsageImageLayout)
			? infoA.m_UsageImageLayout : vk::ImageLayout::eGeneral;
		return result;
	}

	const ResourceUsageVulkanInfo GetUsageInfo(ResourceUsageFlags usageFlags)
	{
		if (usageFlags == 0)
		{
			return s_NoneUsageVulkanInfo;
		}
		ResourceUsageVulkanInfo result{};
		bool init = false;
		for (uint32_t id = 0; id < EResourceUsageIDs::eMax; ++id)
		{
			if ((usageFlags & (1 << id)) != 0)
			{
				if (!init)
				{
					init = true;
					result = s_ResourceUsageToVulkanInfo[id];
				}
				else
				{
					result = Combination(result, s_ResourceUsageToVulkanInfo[id]);
				}
			}
		}
		return result;
	}


	void VulkanBarrierCollector::PushImageBarrier(vk::Image image, ResourceUsageFlags sourceUsage, ResourceUsageFlags destUsage)
	{
		ResourceUsageVulkanInfo sourceInfo = GetUsageInfo(sourceUsage);
		ResourceUsageVulkanInfo destInfo = GetUsageInfo(destUsage);

		auto key = std::make_tuple(sourceInfo.m_UsageStageMask, destInfo.m_UsageStageMask);
		auto found = m_BarrierGroups.find(key);
		if (found == m_BarrierGroups.end())
		{
			m_BarrierGroups.emplace(key, BarrierGroup{});
			found = m_BarrierGroups.find(key);
		}

		found->second.m_Images.push_back(std::make_tuple(sourceInfo, destInfo, image));
	}
	
	void VulkanBarrierCollector::ExecuteBarrier(vk::CommandBuffer commandBuffer)
	{
		ExecuteCurrentQueueBarriers(commandBuffer);
	}

	
	void VulkanBarrierCollector::ExecuteCurrentQueueBarriers(vk::CommandBuffer commandBuffer)
	{
		for (auto& key_value : m_BarrierGroups)
		{
			std::vector<vk::ImageMemoryBarrier> imageBarriers;
			auto key = key_value.first;
			imageBarriers.reserve(key_value.second.m_Images.size());
			for (auto& imgInfo : key_value.second.m_Images)
			{
				ResourceUsageVulkanInfo& sourceInfo = std::get<0>(imgInfo);
				ResourceUsageVulkanInfo& destInfo = std::get<1>(imgInfo);
				vk::Image image = std::get<2>(imgInfo);

				vk::ImageMemoryBarrier newBarrier(
					sourceInfo.m_UsageAccessFlags
					, destInfo.m_UsageAccessFlags
					, sourceInfo.m_UsageImageLayout
					, destInfo.m_UsageImageLayout
					, m_CurrentQueueFamilyIndex
					, m_CurrentQueueFamilyIndex
					, image
					, vulkan_backend::utils::DefaultColorSubresourceRange());
				imageBarriers.push_back(newBarrier);
			}
			commandBuffer.pipelineBarrier(
				std::get<0>(key)
				, std::get<1>(key)
				, {}
				, {}
				, {}
				, imageBarriers);
		}

	}
}