#pragma once
#include <private/include/VulkanApplicationSubobjectBase.h>
#include <RenderInterface/header/CPipelineStateObject.h>
#include <private/include/VulkanIncludes.h>

#include "RenderInterface/header/CShaderModule.h"
#include "RenderInterface/header/CVertexInputDescriptor.h"

namespace graphics_backend
{

	class CPipelineObject final : public BaseApplicationSubobject
	{
	public:
		struct RenderPassInfo
		{
			vk::RenderPass renderPass = nullptr;
			uint32_t subpassId = 0;
		};

		struct ShaderDataLayoutInfo
		{
			vk::PipelineLayout layout;
		};

		CPipelineObject(CVulkanApplication& owner) : BaseApplicationSubobject(owner) {};

		void BuildPipelineObject(
			CPipelineStateObject const& srcPSO
			, CVertexInputDescriptor const& vertexInputs
			, CGraphicsShaderStates const& shaderStates
			, RenderPassInfo const& renderPassInfo
			, ShaderDataLayoutInfo const& pipelineLayout);
	protected:
		vk::Pipeline m_GraphicsPipeline = nullptr;
	};
}
