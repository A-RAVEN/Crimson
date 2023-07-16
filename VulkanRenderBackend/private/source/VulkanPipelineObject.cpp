#include <private/include/pch.h>
#include <private/include/VulkanPipelineObject.h>

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

	void CPipelineObject::BuildPipelineObject(CPipelineStateObject const& srcPSO,
		CVertexInputDescriptor const& vertexInputs, CGraphicsShaderStates const& shaderStates,
		RenderPassInfo const& renderPassInfo)
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



		//vk::GraphicsPipelineCreateInfo graphicsPipeCreateInfo(
		//	
		//)
	}
}
