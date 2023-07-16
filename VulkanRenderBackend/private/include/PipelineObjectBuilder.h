#pragma once
#include <private/include/VulkanApplicationSubobjectBase.h>
#include <RenderInterface/header/CPipelineStateObject.h>
#include <private/include/VulkanIncludes.h>
namespace graphics_backend
{

	class CPipelineObjectBuilder : public ApplicationSubobjectBase
	{
	public:

		struct RenderPassInfo
		{
			vk::RenderPass renderPass = nullptr;
			uint32_t subpassId = 0;
		};

		struct Input

		void BuildPipelineObject(CPipelineStateObject const& srcPSO);
	protected:
		virtual void Initialize_Internal(CVulkanApplication const* owningApplication) {};
		virtual void Release_Internal() {};

		vk::Pipeline m_GraphicsPipeline = nullptr;
	};
}