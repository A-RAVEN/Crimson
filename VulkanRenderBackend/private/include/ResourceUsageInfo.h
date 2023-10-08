#pragma once
#include <cstdint>
#include <SharedTools/header/uenum.h>



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
		eDepthStencilAttachmentID,
		eDepthStencilReadonlyID,

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
		eDepthStencilAttachment = 1 << eDepthStencilAttachmentID,
		eDepthStencilReadonly = 1 << eDepthStencilReadonlyID,

		ePresent = 1 << ePresentID,
	};

	using ResourceUsageFlags = uenum::EnumFlags<ResourceUsage>;

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

	const ResourceUsageVulkanInfo GetUsageInfo(ResourceUsageFlags usageFlags);
}

template<>
struct uenum::TEnumTraits<graphics_backend::ResourceUsage>
{
	static constexpr bool is_bitmask = true;
};