#include <unordered_map>
#include <private/include/pch.h>
#include <private/include/VulkanPipelineObject.h>

#include "private/include/CShaderModule_Vulkan.h"

namespace graphics_backend
{
	vk::Format VertexInputFormatToVkFormat(VertexInputFormat inFormat)
	{
		switch (inFormat)
		{
		case VertexInputFormat::eR32_SFloat: return vk::Format::eR32Sfloat;
		case VertexInputFormat::eR32G32_SFloat: return vk::Format::eR32G32Sfloat;
		case VertexInputFormat::eR32G32B32_SFloat: return vk::Format::eR32G32B32Sfloat;
		case VertexInputFormat::eR32G32B32A32_SFloat: return vk::Format::eR32G32B32A32Sfloat;
		case VertexInputFormat::eR32_UInt: return vk::Format::eR32Uint;
		case VertexInputFormat::eR32_SInt: return vk::Format::eR32Sint;
		default: return vk::Format::eR32Sfloat;
		}
	}

	vk::PrimitiveTopology ETopologyToVkTopology(ETopology inTopology)
	{
		switch (inTopology)
		{
		case ETopology::eTriangleList: return vk::PrimitiveTopology::eTriangleList;
		case ETopology::eTriangleStrip: return vk::PrimitiveTopology::eTriangleStrip;
		case ETopology::ePointList: return vk::PrimitiveTopology::ePointList;
		case ETopology::eLineList: return vk::PrimitiveTopology::eLineList;
		case ETopology::eLineStrip: return vk::PrimitiveTopology::eLineStrip;
		default: return vk::PrimitiveTopology::eTriangleList;
		}
	}

	vk::PolygonMode EPolygonModeTranslate(EPolygonMode inPolygonMode)
	{
		switch (inPolygonMode)
		{
		case EPolygonMode::eFill: return vk::PolygonMode::eFill;
		case EPolygonMode::eLine: return vk::PolygonMode::eLine;
		case EPolygonMode::ePoint: return vk::PolygonMode::ePoint;
		default: return vk::PolygonMode::eFill;
		}
	}

	vk::CullModeFlags ECullModeTranslate(ECullMode inCullMode)
	{
		switch (inCullMode)
		{
		case ECullMode::eBack: return vk::CullModeFlagBits::eBack;
		case ECullMode::eFront: return vk::CullModeFlagBits::eFront;
		case ECullMode::eNone: return vk::CullModeFlagBits::eNone;
		case ECullMode::eAll: return vk::CullModeFlagBits::eFrontAndBack;
		default: return vk::CullModeFlagBits::eNone;
		}
	}

	vk::FrontFace EFrontFaceTranslate(EFrontFace inFrontFace)
	{
		switch (inFrontFace)
		{
		case EFrontFace::eClockWise: return vk::FrontFace::eClockwise;
		case EFrontFace::eCounterClockWise: return vk::FrontFace::eCounterClockwise;
		default: return vk::FrontFace::eClockwise;
		}
	}

	vk::StencilOp EStencilOpTranslate(EStencilOp inStencilOp)
	{
		switch(inStencilOp)
		{
		case EStencilOp::eKeep: return vk::StencilOp::eKeep;
		case EStencilOp::eReplace: return vk::StencilOp::eReplace;
		case EStencilOp::eZero: return vk::StencilOp::eZero;
		default: return vk::StencilOp::eKeep;
		}
	}

	vk::CompareOp ECompareOpTranslate(ECompareOp inCompareOp)
	{
		switch (inCompareOp)
		{
		case ECompareOp::eAlways: return vk::CompareOp::eAlways;
		case ECompareOp::eNever: return vk::CompareOp::eNever;
		case ECompareOp::eLEqual: return vk::CompareOp::eLessOrEqual;
		case ECompareOp::eGEqual: return vk::CompareOp::eGreaterOrEqual;
		case ECompareOp::eLess: return vk::CompareOp::eLess;
		case ECompareOp::eGreater: return vk::CompareOp::eGreater;
		case ECompareOp::eEqual: return vk::CompareOp::eEqual;
		case ECompareOp::eUnequal: return vk::CompareOp::eNotEqual;
		default: return vk::CompareOp::eAlways;
		}
	}

	vk::BlendFactor EBlendFactorTranslate(EBlendFactor inBlendFactor)
	{
		switch(inBlendFactor)
		{
		case EBlendFactor::eZero: return vk::BlendFactor::eZero;
		case EBlendFactor::eOne: return vk::BlendFactor::eOne;
		case EBlendFactor::eSrcAlpha: return vk::BlendFactor::eSrcAlpha;
		case EBlendFactor::eOneMinusSrcAlpha: return vk::BlendFactor::eOneMinusSrcAlpha;
		case EBlendFactor::eDstAlpha: return vk::BlendFactor::eDstAlpha;
		case EBlendFactor::eOneMinusDstAlpha: return vk::BlendFactor::eOneMinusDstAlpha;
		case EBlendFactor::eSrcColor: return vk::BlendFactor::eSrcColor;
		case EBlendFactor::eOneMinusSrcColor: return vk::BlendFactor::eOneMinusSrcColor;
		case EBlendFactor::eDstColor: return vk::BlendFactor::eDstColor;
		case EBlendFactor::eOneMinusDstColor: return vk::BlendFactor::eOneMinusDstColor;
		default: return vk::BlendFactor::eZero;
		}
	}

	vk::BlendOp EBlendOpTranslate(EBlendOp inBlendOp)
	{
		switch(inBlendOp)
		{
		case EBlendOp::eAdd: return vk::BlendOp::eAdd;
		case EBlendOp::eSubtract: return vk::BlendOp::eSubtract;
		case EBlendOp::eReverseSubtract: return vk::BlendOp::eReverseSubtract;
		case EBlendOp::eMin: return vk::BlendOp::eMin;
		case EBlendOp::eMax: return vk::BlendOp::eMax;
		default: return vk::BlendOp::eAdd;
		}
	}

	vk::ColorComponentFlags EColorChannelMaskTranslate(EColorChannelMaskFlags inColorMask)
	{
		vk::ColorComponentFlags result = vk::ColorComponentFlags(0);
		if(inColorMask & EColorChannelMask::eR)
		{
			result |= vk::ColorComponentFlagBits::eR;
		}
		if (inColorMask & EColorChannelMask::eG)
		{
			result |= vk::ColorComponentFlagBits::eG;
		}
		if (inColorMask & EColorChannelMask::eB)
		{
			result |= vk::ColorComponentFlagBits::eB;
		}
		if (inColorMask & EColorChannelMask::eA)
		{
			result |= vk::ColorComponentFlagBits::eA;
		}
		return result;
	}

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
