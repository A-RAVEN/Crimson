#include <private/include/pch.h>
#include <private/include/CVulkanApplication.h>
#include <private/include/InterfaceTranslator.h>
#include <private/include/VulkanPipelineObject.h>

namespace graphics_backend
{
	
	void PopulateVertexInputStates(std::vector<vk::VertexInputBindingDescription>& inoutVertexBindingDescs
		, std::vector<vk::VertexInputAttributeDescription>& inoutVertexAttributeDescs
		, CVertexInputDescriptor const& vertexInputs)
	{
		uint32_t attribute_count = 0;
		for(auto& desc : vertexInputs.m_PrimitiveDescriptions)
		{
			attribute_count += static_cast<uint32_t>(std::get<1>(desc).size());
		}
		inoutVertexBindingDescs.clear();
		inoutVertexAttributeDescs.clear();
		inoutVertexBindingDescs.reserve(vertexInputs.m_PrimitiveDescriptions.size());
		inoutVertexAttributeDescs.reserve(attribute_count);

		for (uint32_t bindingId = 0; bindingId < vertexInputs.m_PrimitiveDescriptions.size(); ++bindingId)
		{
			auto& comp = vertexInputs.m_PrimitiveDescriptions[bindingId];
			vk::VertexInputBindingDescription newInputBinding(
				bindingId
				, std::get<0>(comp)
				, std::get<2>(comp) ? vk::VertexInputRate::eInstance : vk::VertexInputRate::eVertex);
			inoutVertexBindingDescs.push_back(newInputBinding);

			auto& attribArray = std::get<1>(comp);
			for(uint32_t locationId = 0; locationId < attribArray.size(); ++locationId)
			{
				auto& attribData = attribArray[locationId];
				vk::VertexInputAttributeDescription newAttribDesc(
					attribData.attributeIndex
					, bindingId
					, VertexInputFormatToVkFormat(attribData.format)
					, attribData.offset
				);
				inoutVertexAttributeDescs.push_back(newAttribDesc);
			}
		}
	}

	vk::PipelineInputAssemblyStateCreateInfo PopulateInputAssemblyInfo(CVertexInputDescriptor const& vertexInputs)
	{
		vk::PipelineInputAssemblyStateCreateInfo result(
			{}
			, ETopologyToVkTopology(vertexInputs.assemblyStates.topology));
		return result;
	}

	vk::PipelineRasterizationStateCreateInfo PopulateRasterizationStateInfo(CPipelineStateObject const& srcPSO)
	{
		auto& rastState = srcPSO.rasterizationStates;
		vk::PipelineRasterizationStateCreateInfo result(
			{}
			, rastState.enableDepthClamp
			, rastState.discardRasterization
			, EPolygonModeTranslate(rastState.polygonMode)
			, ECullModeTranslate(rastState.cullMode)
			, EFrontFaceTranslate(rastState.frontFace)
			, false,{}, {}, {}
			, rastState.lineWidth
		);
		return result;
	}

	vk::PipelineMultisampleStateCreateInfo PopulateMultiSampleStateInfo(MultiSampleStates const& msState)
	{
		vk::PipelineMultisampleStateCreateInfo result{
			{}
			, ESampleCountTranslate(msState.msCount)
		};
		return result;
	}

	void PopulateStencilOpState(DepthStencilStates::StencilStates const& stencilState, vk::StencilOpState& inoutVulkanStencilOp)
	{
		inoutVulkanStencilOp.failOp = EStencilOpTranslate(stencilState.failOp);
		inoutVulkanStencilOp.passOp = EStencilOpTranslate(stencilState.passOp);
		inoutVulkanStencilOp.depthFailOp = EStencilOpTranslate(stencilState.depthFailOp);
		inoutVulkanStencilOp.compareOp = ECompareOpTranslate(stencilState.compareOp);
		inoutVulkanStencilOp.compareMask = stencilState.compareMask;
		inoutVulkanStencilOp.writeMask = stencilState.writeMask;
		inoutVulkanStencilOp.reference = stencilState.reference;
	}

	vk::PipelineDepthStencilStateCreateInfo PopulateDepthStencilStateInfo(CPipelineStateObject const& srcPSO)
	{
		vk::PipelineDepthStencilStateCreateInfo result{};
		auto& depthStencilState = srcPSO.depthStencilStates;
		result.depthTestEnable = depthStencilState.depthTestEnable;
		result.depthWriteEnable = depthStencilState.depthWriteEnable;
		result.depthCompareOp = ECompareOpTranslate(depthStencilState.depthCompareOp);
		result.depthBoundsTestEnable = false;
		result.maxDepthBounds = 1.0f;
		result.minDepthBounds = 0.0f;
		result.stencilTestEnable = depthStencilState.stencilTestEnable;
		PopulateStencilOpState(depthStencilState.stencilStateFront, result.front);
		PopulateStencilOpState(depthStencilState.stencilStateBack, result.back);
		return result;
	}

	vk::PipelineColorBlendStateCreateInfo PopulateColorBlendStateInfo(
		CPipelineStateObject const& srcPSO
		, RenderPassObject const* pRenderPassObj
		, std::vector<vk::PipelineColorBlendAttachmentState>& inoutBlendAttachmentStates)
	{
		auto& colorAttachments = srcPSO.colorAttachments;
		vk::PipelineColorBlendStateCreateInfo result{};
		uint32_t attachmentCount = std::min(
			static_cast<uint32_t>(colorAttachments.attachmentBlendStates.size())
			, pRenderPassObj->GetAttachmentCount());
		inoutBlendAttachmentStates.resize(attachmentCount);
		for(uint32_t i = 0; i < attachmentCount; ++i)
		{
			auto const& srcInfo = colorAttachments.attachmentBlendStates[i];
			auto& dstInfo = inoutBlendAttachmentStates[i];
			dstInfo.blendEnable = srcInfo.blendEnable;
			dstInfo.srcColorBlendFactor = EBlendFactorTranslate(srcInfo.sourceColorBlendFactor);
			dstInfo.dstColorBlendFactor = EBlendFactorTranslate(srcInfo.destColorBlendFactor);
			dstInfo.srcAlphaBlendFactor = EBlendFactorTranslate(srcInfo.sourceAlphaBlendFactor);
			dstInfo.dstAlphaBlendFactor = EBlendFactorTranslate(srcInfo.destAlphaBlendFactor);
			dstInfo.colorBlendOp = EBlendOpTranslate(srcInfo.colorBlendOp);
			dstInfo.alphaBlendOp = EBlendOpTranslate(srcInfo.alphaBlendOp);
			dstInfo.colorWriteMask = EColorChannelMaskTranslate(srcInfo.channelMask);
		}
		result.setAttachments(inoutBlendAttachmentStates);
		return result;
	}

	void PopulateShaderStages(ShaderStateDescriptor const& shaderStates
		, std::vector<vk::PipelineShaderStageCreateInfo>& inoutShaderStages)
	{
		auto vertexModdule = shaderStates.vertexShader->GetShaderModule();
		auto fragmentModdule = shaderStates.fragmentShader->GetShaderModule();
		inoutShaderStages.clear();
		inoutShaderStages.push_back(vk::PipelineShaderStageCreateInfo{
			{}//vk::PipelineShaderStageCreateFlagBits::eAllowVaryingSubgroupSize
				, vk::ShaderStageFlagBits::eVertex
				, vertexModdule
				, shaderStates.vertexShader->GetEntryPointName().c_str()
		});
		inoutShaderStages.push_back(vk::PipelineShaderStageCreateInfo{
			{}//vk::PipelineShaderStageCreateFlagBits::eAllowVaryingSubgroupSize
				, vk::ShaderStageFlagBits::eFragment
				, fragmentModdule
				, shaderStates.fragmentShader->GetEntryPointName().c_str()
		});
	}

	void CPipelineObject::Create(CPipelineObjectDescriptor const& pipelineObjectDescriptor)
	{
		//Vertex States
		std::vector<vk::VertexInputBindingDescription> vertexBindingDescriptions;
		std::vector<vk::VertexInputAttributeDescription> vertexAttributeDescriptions;
		PopulateVertexInputStates(
			vertexBindingDescriptions
			, vertexAttributeDescriptions
			, pipelineObjectDescriptor.vertexInputs);

		vk::PipelineVertexInputStateCreateInfo vertexStateCreateInfo({}, vertexBindingDescriptions, vertexAttributeDescriptions);

		//Input Assembly
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = PopulateInputAssemblyInfo(pipelineObjectDescriptor.vertexInputs);

		//Rasterization States
		vk::PipelineRasterizationStateCreateInfo rasterizationInfo = PopulateRasterizationStateInfo(pipelineObjectDescriptor.pso);

		//MultiSample States
		vk::PipelineMultisampleStateCreateInfo multisampleInfo = PopulateMultiSampleStateInfo(pipelineObjectDescriptor.pso.multiSampleStates);
		
		//Depth Stencil
		vk::PipelineDepthStencilStateCreateInfo depthStencilState = PopulateDepthStencilStateInfo(pipelineObjectDescriptor.pso);

		//Color Attachment State
		std::vector<vk::PipelineColorBlendAttachmentState> inoutBlendAttachmentStates;
		vk::PipelineColorBlendStateCreateInfo colorBlendState = PopulateColorBlendStateInfo(
			pipelineObjectDescriptor.pso
			, pipelineObjectDescriptor.renderPassObject.get()
			, inoutBlendAttachmentStates);

		//Shader Stages
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		PopulateShaderStages(pipelineObjectDescriptor.shaderState, shaderStages);

		//TODO Populate Shader Binding Layout Info
		vk::PipelineLayoutCreateInfo layoutCreateInfo{{}, pipelineObjectDescriptor.descriptorSetLayouts, {}};
		m_PipelineLayout = GetDevice().createPipelineLayout(layoutCreateInfo);

		std::array<vk::Viewport, 1> dummyViewports = { vk::Viewport{0, 0, 1, 1, 0, 1} };
		std::array<vk::Rect2D, 1> dummySissors = { vk::Rect2D{{0, 0}, {1, 1}} };
		vk::PipelineViewportStateCreateInfo viewportStateInfo{
			{}, dummyViewports, dummySissors };

		std::array<vk::DynamicState, 2> dynamicStates{
			vk::DynamicState::eViewport
			, vk::DynamicState::eScissor
		};
		vk::PipelineDynamicStateCreateInfo dynamicStateInfo{
			{}, dynamicStates
		};

		vk::GraphicsPipelineCreateInfo graphicsPipeCreateInfo{};
		graphicsPipeCreateInfo.layout = m_PipelineLayout;
		graphicsPipeCreateInfo.setPColorBlendState(&colorBlendState);
		graphicsPipeCreateInfo.setStages(shaderStages);
		graphicsPipeCreateInfo.setPVertexInputState(&vertexStateCreateInfo);
		graphicsPipeCreateInfo.setPInputAssemblyState(&inputAssemblyInfo);
		graphicsPipeCreateInfo.setPRasterizationState(&rasterizationInfo);
		graphicsPipeCreateInfo.setPDepthStencilState(&depthStencilState);
		graphicsPipeCreateInfo.setPMultisampleState(&multisampleInfo);
		graphicsPipeCreateInfo.setRenderPass(pipelineObjectDescriptor.renderPassObject->GetRenderPass());
		graphicsPipeCreateInfo.setSubpass(pipelineObjectDescriptor.subpassIndex);
		graphicsPipeCreateInfo.setPViewportState(&viewportStateInfo);
		graphicsPipeCreateInfo.setPDynamicState(&dynamicStateInfo);
		m_GraphicsPipeline = GetDevice().createGraphicsPipeline(nullptr, graphicsPipeCreateInfo).value;
	}
}
