#pragma once
#include <private/include/VulkanIncludes.h>
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
		eComputeReadWriteID,

		eColorAttachmentOutputID,

		ePresentID,
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
		eComputeReadWrite = 1 << eComputeReadWriteID,

		eColorAttachmentOutput = 1 << eColorAttachmentOutputID,

		ePresent = 1 << ePresentID,
	};

	using ResourceUsageFlags = uint32_t;

	struct ResourceUsageVulkanInfo
	{
	public:
		vk::PipelineStageFlags m_UsageStageMask;
		vk::AccessFlags m_UsageAccessFlags;
		vk::ImageLayout m_UsageLayout;
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
		VulkanBarrierCollector(uint32_t currentQueueFamilyIndex);

		void PushImageBarrier(vk::Image image
			, ResourceUsageFlags sourceUsage
			, ResourceUsageFlags destUsage);

		void PushImageReleaseBarrier(vk::Image image
			, ResourceUsageFlags sourceUsage
			, ResourceUsageFlags destUsage
			, uint32_t destQueueFamily);

		void PushImageAquireBarrier(vk::Image image
			, ResourceUsageFlags sourceUsage
			, ResourceUsageFlags destUsage
			, uint32_t sourceQueueFamily);

		void ExecuteBarrier(vk::CommandBuffer commandBuffer);

	private:
		uint32_t m_CurrentQueueFamilyIndex;

	};
}