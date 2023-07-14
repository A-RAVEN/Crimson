#pragma once
#include <private/include/VulkanIncludes.h>
namespace graphics_backend
{
	enum class ResourceUsage
	{
		eDontCare = 0,

		eTransferSource,
		eTransferDest,

		eVertexRead,
		eVertexWrite,
		eFragmentRead,
		eFragmentWrite,
		eComputeRead,
		eComputeWrite,
		eComputeReadWrite,

		ePresent,
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
		void PushImageBarrier(vk::Image image
			, ResourceUsage sourceUsage
			, ResourceUsage destUsage
			, ReourceQueueFamily sourceQueue
			, ReourceQueueFamily destQueue);

		void Execute(vk::CommandBuffer commandBuffer);
	};
}