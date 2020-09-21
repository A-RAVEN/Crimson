#pragma once
#include "Generals.h"
#include "DescriptorSets.h"
#include "PipelineEnums.h"
#include <string>
#include <vector>

namespace Crimson
{

	class VertexInputDescriptor
	{
	public:
		EVertexInputMode					m_VertexInputMode;
		std::vector<EDataType>				m_DataTypes;
		//only needed in dx12 backend
		std::vector<std::string>	m_Sematics;
	};

	class BlendSetting
	{
	public:
		EBlendFactor	m_SrcFactor;
		EBlendFactor	m_DstFactor;
		EBlendOp		m_BlendOp;
		BlendSetting(EBlendFactor src_factor = EBlendFactor::E_ONE, EBlendFactor dst_factor = EBlendFactor::E_ZERO, EBlendOp op = EBlendOp::E_ADD) :
			m_SrcFactor(src_factor),
			m_DstFactor(dst_factor),
			m_BlendOp(op)
		{}
		bool inline IsNoBlendSetting() const
		{
			return (m_SrcFactor == EBlendFactor::E_ONE) && (m_DstFactor == EBlendFactor::E_ZERO) && (m_BlendOp == EBlendOp::E_ADD);
		}
	};

	class ShaderModule : public IObject
	{
	public:
	protected:
		EShaderType m_ShaderType;
	};

	class PushConstantRange
	{
	public:
		std::vector<EShaderType> m_ShaderTypes;
		uint64_t m_Offset;
		uint64_t m_Size;
		PushConstantRange() : m_Offset(0), m_Size(0) {}
		PushConstantRange(std::vector<EShaderType> const& shader_type, uint64_t offset, uint64_t size) : m_ShaderTypes(shader_type), m_Offset(offset), m_Size(size) {}
	};

	using PShaderModule = ShaderModule*;

	class GraphicsPipeline : public IObject
	{
	public:
		virtual void LoadShaderSource(char const* src_code, size_t src_size, EShaderType shader_type) = 0;
		virtual void LoadShaderModule(PShaderModule shader_module) = 0;
		std::vector<std::pair<uint32_t, PDescriptorSetLayout>>		m_DescriptorSetLayouts;
		std::vector<VertexInputDescriptor>		m_VertexInputs;
		std::vector<BlendSetting>				m_ColorBlendSettings;
		std::vector<BlendSetting>				m_AlphaBlendSettings;
		std::vector<PushConstantRange>			m_PushConstants;
		std::vector<PShaderModule>				m_ShaderModules;

		ECullMode								m_CullMode;
		EPolygonMode							m_PolygonMode;
		ETopology								m_Topology;

		EDepthTestRule							m_DepthRule;
		ECompareMode							m_DepthCompareMode;
		EStencilRule							m_StencilRule;

		uint32_t m_SubpassInputAttachmentBindPoint;
		uint32_t m_TesselationPatchSize;
		uint32_t m_MultiSampleShadingNum;

	protected:
		GraphicsPipeline() :
			m_CullMode(ECullMode::E_CULL_NONE),
			m_PolygonMode(EPolygonMode::E_POLYGON_MODE_FILL),
			m_Topology(ETopology::E_TOPOLOGY_TRIANGLE_LIST),
			m_DepthRule(EDepthTestRule::E_DEPTH_TEST_DISABLED),
			m_DepthCompareMode(ECompareMode::E_COMPARE_MODE_LESS),
			m_StencilRule(EStencilRule::E_STENCIL_DISABLED),
			m_SubpassInputAttachmentBindPoint(0),
			m_TesselationPatchSize(0),
			m_MultiSampleShadingNum(0)
		{}

	};
	using PGraphicsPipeline = GraphicsPipeline*;

	class GraphicsPipelineInstanciationInfo
	{
	public:
		std::vector<std::pair<uint32_t, PDescriptorSetLayout>>		m_DescriptorSetLayouts;
		std::vector<VertexInputDescriptor>		m_VertexInputs;
		std::vector<BlendSetting>				m_ColorBlendSettings;
		std::vector<BlendSetting>				m_AlphaBlendSettings;
		std::vector <std::pair<EShaderType, PShaderModule>> m_Shaders;

		ECullMode								m_CullMode;
		EPolygonMode							m_PolygonMode;
		ETopology								m_Topology;

		EDepthTestRule							m_DepthRule;
		ECompareMode							m_DepthCompareMode;
		EStencilRule							m_StencilRule;

		uint32_t m_SubpassInputAttachmentBindPoint;
		uint32_t m_TesselationPatchSize;
		uint32_t m_MultiSampleShadingNum;

		GraphicsPipelineInstanciationInfo() :
			m_CullMode(ECullMode::E_CULL_NONE),
			m_PolygonMode(EPolygonMode::E_POLYGON_MODE_FILL),
			m_Topology(ETopology::E_TOPOLOGY_TRIANGLE_LIST),
			m_DepthRule(EDepthTestRule::E_DEPTH_TEST_DISABLED),
			m_DepthCompareMode(ECompareMode::E_COMPARE_MODE_LESS),
			m_StencilRule(EStencilRule::E_STENCIL_DISABLED),
			m_SubpassInputAttachmentBindPoint(0),
			m_TesselationPatchSize(0),
			m_MultiSampleShadingNum(0)
		{}
	protected:
	};
}