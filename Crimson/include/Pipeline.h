#pragma once
#include "Generals.h"

namespace Crimson
{
	enum class EShaderResourceType : uint8_t
	{
		E_SHADER_UNIFORM_BUFFER = 0,
		E_SHADER_IMAGE_SAMPLER,
		E_SHADER_STORAGE_IMAGE,
		E_SHADER_TYPE_STORAGE_BUFFER,
		E_SHADER_TYPE_INPUT_ATTACHMENT,
		E_SHADER_BINDING_MAX,
	};

	enum class EShaderType : uint8_t
	{
		E_SHADER_TYPE_VERTEX = 0,
		E_SHADER_TYPE_TESSCTR,
		E_SHADER_TYPE_TESSEVL,
		E_SHADER_TYPE_GEOMETRY,
		E_SHADER_TYPE_FRAGMENT,
		E_SHADER_TYPE_COMPUTE,
		//nvidia mesh shader
		E_SHADER_TYPE_TASK_NV,
		E_SHADER_TYPE_MESH_NV,
		E_SHADER_TYPE_MAX,
	};

	class DescriptorSet : IObject
	{
	public:
		virtual void WriteDescriptorSetBuffer() {};
		virtual void WriteDescriptorSetImage() {};
		virtual void EndWriteDescriptorSet() {};
	};
	using PDescriptorSet = DescriptorSet*;

	class ShaderBinding
	{
	public:
		uint32_t					m_BindingPoint;
		EShaderResourceType			m_ResourceType;
		std::vector<EShaderType>	m_ShaderTypes;
		uint32_t					m_Num;
		ShaderBinding(uint32_t bindpoint = 0, EShaderResourceType resource_type = EShaderResourceType::E_SHADER_BINDING_MAX, std::vector<EShaderType> const& shader_types = {}, uint32_t array_number = 1) :
			m_BindingPoint(bindpoint),
			m_ResourceType(resource_type),
			m_ShaderTypes(shader_types),
			m_Num(array_number)
		{}
	};

	class DescriptorSetLayout : IObject
	{
	public:
		virtual PDescriptorSet AllocDescriptorSet() = 0;
		virtual void BuildLayout() = 0;
		std::vector<ShaderBinding> m_Bindings;
	};
	using PDescriptorSetLayout = DescriptorSetLayout*;

	enum class EVertexInputMode : uint8_t
	{
		E_VERTEX_INPUT_PER_VERTEX = 0,
		E_VERTEX_INPUT_PER_INSTANCE,
		E_VERTEX_INPUT_MODE_MAX,
	};

	enum class EDataType : uint16_t
	{
		EINT = 0,
		EUINT,
		EFLOAT,
		EVEC2,
		EVEC3,
		EVEC4,
		EIVEC2,
		EIVEC3,
		EIVEC4,
		EUVEC2,
		EUVEC3,
		EUVEC4,
		EMAT4,
		ERGBA8,
		EDATA_TYPE_MAX,
	};

	enum class EBlendFactor : uint8_t
	{
		E_ONE = 0,
		E_ZERO,
		E_SRC_ALPHA,
		E_DST_ALPHA,
		E_ONE_MINUS_SRC_ALPHA,
		E_ONE_MINUS_DST_ALPHA,
	};

	enum class EBlendOp : uint8_t
	{
		E_ADD = 0,
		E_MINUS,
		E_SUBTRACT,
		E_MULTIPLY,
	};

	enum class EDepthTestRule : uint8_t
	{
		E_DEPTH_TEST_DISABLED = 0,
		E_DEPTH_TEST_ENABLED,
		E_DEPTH_TEST_READ_ONLY,
		E_DEPTH_TEST_MAX
	};

	enum class ECompareMode : uint8_t
	{
		E_COMPARE_MODE_LESS = 0,
		E_COMPARE_MODE_LESS_OR_EQUAL,
		E_COMPARE_MODE_GREATER,
		E_COMPARE_MODE_MAX
	};

	enum class EStencilRule : uint8_t
	{
		E_STENCIL_DISABLED = 0,
		E_STENCIL_WRITE,
		E_STENCIL_TEST,
		E_STENCIL_INVTEST,
		E_STENCIL_MAX
	};

	class VertexInputDescriptor
	{
		EVertexInputMode		m_VertexInputMode;
		std::vector<EDataType>	m_DataType;
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
	};

	class GraphicsPipeline : IObject
	{
	public:
		std::vector<std::pair<uint32_t, PDescriptorSetLayout>>		m_DescriptorSetLayouts;
		std::vector<VertexInputDescriptor>		m_VertexInputs;
		std::vector<BlendSetting>				m_ColorBlendSettings;
		std::vector<BlendSetting>				m_AlphaBlendSettings;

		ECullMode								m_CullMode;
		EPolygonMode							m_PolygonMode;
		ETopology								m_Topology;

		EDepthTestRule							m_DepthRule;
		ECompareMode							m_DepthCompareMode;
		EStencilRule							m_StencilRule;

		uint32_t m_SubpassInputAttachmentBindPoint;
		uint32_t m_TesselationPatchSize;
		uint32_t m_MultiSampleShadingNum;

		//uint32_t m_PushConstantSize = 0;
		//ShaderTypes m_PushConstantShaderTypes = 0;
	private:
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
}