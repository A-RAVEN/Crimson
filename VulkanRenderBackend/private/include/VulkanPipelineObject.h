#pragma once
#include <private/include/VulkanApplicationSubobjectBase.h>
#include <RenderInterface/header/CPipelineStateObject.h>
#include <private/include/VulkanIncludes.h>
#include <private/include/CShaderModuleObject.h>
#include "RenderInterface/header/CShaderModule.h"
#include "RenderInterface/header/CVertexInputDescriptor.h"
#include "RenderPassObject.h"

namespace graphics_backend
{
	struct ShaderStateDescriptor
	{
	public:
		std::shared_ptr<CShaderModuleObject> vertexShader = nullptr;
		std::shared_ptr <CShaderModuleObject> fragmentShader = nullptr;

		bool operator==(ShaderStateDescriptor const& rhs) const
		{
			return vertexShader == rhs.vertexShader
				&& fragmentShader == rhs.fragmentShader;
		}

		template <class HashAlgorithm>
		friend void hash_append(HashAlgorithm& h, ShaderStateDescriptor const& shaderstate_desc) noexcept
		{
			hash_append(h, static_cast<size_t>(shaderstate_desc.vertexShader.get()));
			hash_append(h, static_cast<size_t>(shaderstate_desc.fragmentShader.get()));
		}
	};

	struct CPipelineObjectDescriptor
	{
		CPipelineStateObject pso{};
		CVertexInputDescriptor vertexInputs{};
		ShaderStateDescriptor shaderState{};
		std::shared_ptr<RenderPassObject> renderPassObject = nullptr;
		uint32_t subpassIndex = 0;

		bool operator==(CPipelineObjectDescriptor const& rhs) const
		{
			return pso == rhs.pso
				&& vertexInputs == rhs.vertexInputs
				&& shaderState == rhs.shaderState
				&& renderPassObject == rhs.renderPassObject
				&& subpassIndex == rhs.subpassIndex;
		}

		template <class HashAlgorithm>
		friend void hash_append(HashAlgorithm& h, CPipelineObjectDescriptor const& pipeline_desc) noexcept
		{
			hash_append(h, pipeline_desc.pso);
			hash_append(h, pipeline_desc.vertexInputs);
			hash_append(h, pipeline_desc.shaderState);
			hash_append(h, static_cast<size_t>(pipeline_desc.renderPassState.get()));
			hash_append(h, pipeline_desc.subpassIndex);
		}
	};

	class CPipelineObject final : public BaseApplicationSubobject
	{
	public:
		CPipelineObject(CVulkanApplication& owner) : BaseApplicationSubobject(owner) {};
		void Create(CPipelineObjectDescriptor const& pipelineObjectDescriptor);
	protected:
		vk::Pipeline m_GraphicsPipeline = nullptr;

		vk::PipelineLayout m_PipelineLayout = nullptr;
	};

	using PipelineObjectDic = HashPool<CPipelineObjectDescriptor, CPipelineObject>;
}
