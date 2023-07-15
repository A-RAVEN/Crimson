#pragma once
#include <private/include/VulkanIncludes.h>
#include <map>
#include <deque>
namespace graphics_backend
{
	enum EResourceUsageIDs
	{
		eTransferSourceID = 0,
		eTransferDestID,

		eVertexReadID,
		eVertexWriteID,
		eFragmentReadID,
		eFragmentWriteID,
		eComputeReadID,
		eComputeWriteID,

		eColorAttachmentOutputID,

		ePresentID,

		eMax,
	};

	enum ResourceUsage : uint32_t
	{
		eDontCare = 0,

		eTransferSource = 1 << eTransferSourceID,
		eTransferDest = 1 << eTransferDestID,

		eVertexRead = 1 << eVertexReadID,
		eVertexWrite = 1 << eVertexWriteID,
		eFragmentRead = 1 << eFragmentReadID,
		eFragmentWrite = 1 << eFragmentWriteID,
		eComputeRead = 1 << eComputeReadID,
		eComputeWrite = 1 << eComputeWriteID,

		eColorAttachmentOutput = 1 << eColorAttachmentOutputID,

		ePresent = 1 << ePresentID,
	};

	using ResourceUsageFlags = uint32_t;

	struct ResourceUsageVulkanInfo
	{
	public:
		vk::PipelineStageFlags m_UsageStageMask;
		vk::AccessFlags m_UsageAccessFlags;
		vk::ImageLayout m_UsageImageLayout;
	};


	enum class ReourceQueueFamily
	{
		eGraphics,
		eComputeDedicate,
		eTransferDedicate,
	};

	class VulkanBarrierCollector
	{
	public:
		VulkanBarrierCollector(uint32_t currentQueueFamilyIndex) : m_CurrentQueueFamilyIndex(currentQueueFamilyIndex){}

		void PushImageBarrier(vk::Image image
			, ResourceUsageFlags sourceUsage
			, ResourceUsageFlags destUsage);

		//void PushImageReleaseBarrier(vk::Image image
		//	, ResourceUsageFlags sourceUsage
		//	, ResourceUsageFlags destUsage
		//	, uint32_t destQueueFamily);

		//void PushImageAquireBarrier(vk::Image image
		//	, ResourceUsageFlags sourceUsage
		//	, ResourceUsageFlags destUsage
		//	, uint32_t sourceQueueFamily);

		void ExecuteBarrier(vk::CommandBuffer commandBuffer);


		struct BarrierGroup
		{
			std::vector<std::tuple<ResourceUsageVulkanInfo, ResourceUsageVulkanInfo, vk::Image>> m_Images;
		};

	private:

		void ExecuteCurrentQueueBarriers(vk::CommandBuffer commandBuffer);


		uint32_t m_CurrentQueueFamilyIndex;
		std::map<std::tuple<vk::PipelineStageFlags, vk::PipelineStageFlags>
			, BarrierGroup> m_BarrierGroups;

		std::map<std::tuple<vk::PipelineStageFlags, vk::PipelineStageFlags, uint32_t>
			, BarrierGroup> m_ReleaseGroups;

		std::map<std::tuple<vk::PipelineStageFlags, vk::PipelineStageFlags, uint32_t>
			, BarrierGroup> m_AquireGroups;
	};
}