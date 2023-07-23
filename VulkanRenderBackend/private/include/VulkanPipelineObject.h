#pragma once
#include <private/include/VulkanApplicationSubobjectBase.h>
#include <RenderInterface/header/CPipelineStateObject.h>
#include <private/include/VulkanIncludes.h>

#include "RenderInterface/header/CShaderModule.h"
#include "RenderInterface/header/CVertexInputDescriptor.h"
#include "RenderPassObject.h"

namespace graphics_backend
{
	struct ShaderStateDescriptor
	{
		ShaderModuleDescritor vertexShader;
		ShaderModuleDescritor fragmentShader;
	};

	struct CPipelineObjectDescriptor
	{
		CPipelineStateObject pso{};
		CVertexInputDescriptor vertexInputs{};
		ShaderStateDescriptor shaderState{};
		RenderPassDescriptor renderPassState{};

		bool operator==(CPipelineObjectDescriptor const& rhs) const
		{
			return pso == rhs.pso
				&& vertexInputs == rhs.vertexInputs
				&& shaderState.vertexShader == rhs.shaderState.vertexShader
				&& shaderState.fragmentShader == rhs.shaderState.fragmentShader
				&& renderPassState == rhs.renderPassState;
		}

		template <class HashAlgorithm>
		friend void hash_append(HashAlgorithm& h, CPipelineObjectDescriptor const& pipeline_desc) noexcept
		{
			hash_append(h, pipeline_desc.pso);
			hash_append(h, pipeline_desc.vertexInputs);
			hash_append(h, pipeline_desc.vertexInputs);
			hash_append(h, pipeline_desc.shaderState);
			hash_append(h, pipeline_desc.renderPassState);
		}
	};

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
