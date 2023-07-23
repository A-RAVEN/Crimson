#include <unordered_map>
#include <private/include/pch.h>
#include <private/include/VulkanPipelineObject.h>
#include <private/include/CShaderModuleObject.h>
#include <private/include/InterfaceTranslator.h>

namespace graphics_backend
{
	
	void PopulateVertexInputStates(std::vector<vk::VertexInputBindingDescription>& inoutVertexBindingDescs
		, std::vector<vk::VertexInputAttributeDescription>& inoutVertexAttributeDescs
		, CVertexInputDescriptor const& vertexInputs)
	{
		uint32_t attribute_count = 0;
		for(auto& desc : vertexInputs.m_PrimitiveDescriptions)
		{
			attribute_count += std::get<1>(desc).size();
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

	void PopulateStencilOpState(CPipelineStateObject::StencilStates const& stencilState, vk::StencilOpState& inoutVulkanStencilOp)
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
		, std::vector<vk::PipelineColorBlendAttachmentState>& inoutBlendAttachmentStates)
	{
		auto& colorAttachments = srcPSO.colorAttachments;
		vk::PipelineColorBlendStateCreateInfo result{};
		uint32_t attachmentCount = std::min(
			static_cast<uint32_t>(colorAttachments.attachmentBlendStates.size())
			, colorAttachments.attachmentCount);
		inoutBlendAttachmentStates.resize(attachmentCount);
		for(uint32_t i = 0; i < attachmentCount; ++i)
		{
			auto const& srcInfo =colorAttachments.attachmentBlendStates[i];
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

	void PopulateShaderStages(CGraphicsShaderStates const& shaderStates
		, std::vector<vk::PipelineShaderStageCreateInfo>& inoutShaderStages)
	{
		auto vertexModdule = static_cast<CShaderModule_Vulkan*>(shaderStates.vertexShader.get())->GetVulkanModule();
		auto fragmentModdule = static_cast<CShaderModule_Vulkan*>(shaderStates.fragmentShader.get())->GetVulkanModule();
		inoutShaderStages.clear();
		inoutShaderStages.push_back(vk::PipelineShaderStageCreateInfo{
			vk::PipelineShaderStageCreateFlagBits::eAllowVaryingSubgroupSize
				, vk::ShaderStageFlagBits::eVertex
				, vertexModdule
				, "Vertex Shader"
		});
		inoutShaderStages.push_back(vk::PipelineShaderStageCreateInfo{
			vk::PipelineShaderStageCreateFlagBits::eAllowVaryingSubgroupSize
				, vk::ShaderStageFlagBits::eFragment
				, fragmentModdule
				, "Fragment Shader"
		});

		std::unordered_map<CPipelineObject, int> test;
	}


	void CPipelineObject::BuildPipelineObject(
		CPipelineStateObject const& srcPSO
		, CVertexInputDescriptor const& vertexInputs
		, CGraphicsShaderStates const& shaderStates
		, RenderPassInfo const& renderPassInfo
		, ShaderDataLayoutInfo const& pipelineLayout)
	{

		//Vertex States
		std::vector<vk::VertexInputBindingDescription> vertexBindingDescriptions;
		std::vector<vk::VertexInputAttributeDescription> vertexAttributeDescriptions;
		PopulateVertexInputStates(
			vertexBindingDescriptions
			, vertexAttributeDescriptions
			, vertexInputs);

		vk::PipelineVertexInputStateCreateInfo vertexStateCreateInfo({}, vertexBindingDescriptions, vertexAttributeDescriptions);

		//Input Assembly
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = PopulateInputAssemblyInfo(vertexInputs);

		//Rasterization States
		vk::PipelineRasterizationStateCreateInfo rasterizationInfo = PopulateRasterizationStateInfo(srcPSO);


		//Depth Stencil
		vk::PipelineDepthStencilStateCreateInfo depthStencilState = PopulateDepthStencilStateInfo(srcPSO);

		//Color Attachment State
		std::vector<vk::PipelineColorBlendAttachmentState> inoutBlendAttachmentStates;
		vk::PipelineColorBlendStateCreateInfo colorBlendState = PopulateColorBlendStateInfo(srcPSO, inoutBlendAttachmentStates);

		//Shader Stages
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		PopulateShaderStages(shaderStates, shaderStages);

		vk::GraphicsPipelineCreateInfo graphicsPipeCreateInfo{};
		//graphicsPipeCreateInfo.flags = vk::PipelineCreateFlagBits::
		graphicsPipeCreateInfo.layout = pipelineLayout.layout;
		graphicsPipeCreateInfo.setPColorBlendState(&colorBlendState);
		graphicsPipeCreateInfo.setStages(shaderStages);
		graphicsPipeCreateInfo.setPVertexInputState(&vertexStateCreateInfo);
		graphicsPipeCreateInfo.setPInputAssemblyState(&inputAssemblyInfo);
		graphicsPipeCreateInfo.setPRasterizationState(&rasterizationInfo);
		graphicsPipeCreateInfo.setPDepthStencilState(&depthStencilState);
		graphicsPipeCreateInfo.setRenderPass(renderPassInfo.renderPass);
		graphicsPipeCreateInfo.setSubpass(renderPassInfo.subpassId);

		m_GraphicsPipeline = GetDevice().createGraphicsPipeline(nullptr, graphicsPipeCreateInfo).value;

		//GetDevice().getPipelineCacheData()
	}
}
