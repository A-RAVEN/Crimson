#pragma once
#include <cstdint>
#include "TEnumCommon.h"
#include <array>

namespace graphics_backend
{


	enum class ECullMode
	{
		eBack = 0,
		eFront,
		eNone,
		eAll,
	};

	enum class EFrontFace
	{
		eClockWise = 0,
		eCounterClockWise,
	};

	enum class EPolygonMode
	{
		eFill = 0,
		eLine,
		ePoint,
	};

	enum class EMultiSampleCount
	{
		e1 = 0,
		e2,
		e4,
		e8,
		e16,
		e32,
		e64,
	};

	enum class EStencilOp
	{
		eKeep = 0,
		eReplace,
		eZero,
	};

	enum class ECompareOp
	{
		eAlways = 0,
		eNever,
		eLEqual,
		eGEqual,
		eLess,
		eGreater,
		eEqual,
		eUnequal,
	};

	enum class EColorChannelMask : uint8_t
	{
		eR = 1,
		eG = 1 << 1,
		eB = 1 << 2,
		eA = 1 << 3,
		eRGBA = eR | eG | eB | eA,
	};

	using EColorChannelMaskFlags = std::underlying_type_t<EColorChannelMask>;

	enum class EBlendFactor
	{
		eZero = 0,
		eOne,
		eSrcAlpha,
		eOneMinusSrcAlpha,
		eDstAlpha,
		eOneMinusDstAlpha,
		eSrcColor,
		eOneMinusSrcColor,
		eDstColor,
		eOneMinusDstColor,
	};

	enum class EBlendOp
	{
		eAdd = 0,
		eSubtract,
		eReverseSubtract,
		eMin,
		eMax,
	};

	/// <summary>
	/// Pipeline States That May Share During One Pass
	/// Pure Data Struct, Not The Real One Used In Backend API
	/// </summary>
	class CPipelineStateObject
	{
	public:
#pragma region Internal Structs

		struct RasterizerStates
		{
		public:
			bool enableDepthClamp = false;
			bool discardRasterization = false;
			ECullMode cullMode = ECullMode::eBack;
			EFrontFace frontFace = EFrontFace::eClockWise;
			EPolygonMode polygonMode = EPolygonMode::eFill;
			float lineWidth = 1.0f;
		};

		struct MultiSampleStates
		{
			EMultiSampleCount msCount = EMultiSampleCount::e1;
		};

		struct StencilStates
		{
			EStencilOp failOp = EStencilOp::eKeep;
			EStencilOp passOp = EStencilOp::eKeep;
			EStencilOp depthFailOp = EStencilOp::eKeep;
			ECompareOp compareOp = ECompareOp::eAlways;
			uint32_t compareMask = 0u;
			uint32_t writeMask = 0u;
			uint32_t reference = 0u;
		};

		struct DepthStencilStates
		{
			bool depthTestEnable = false;
			bool depthWriteEnable = false;
			ECompareOp depthCompareOp = ECompareOp::eGEqual;

			bool stencilTestEnable = false;
			StencilStates stencilStateFront = {};
			StencilStates stencilStateBack = {};
		};

		struct SingleColorAttachmentBlendStates
		{
			bool blendEnable = false;
			EColorChannelMaskFlags channelMask = ToFlags(EColorChannelMask::eRGBA);
			EBlendFactor sourceColorBlendFactor = EBlendFactor::eOne;
			EBlendFactor destColorBlendFactor = EBlendFactor::eZero;
			EBlendFactor sourceAlphaBlendFactor = EBlendFactor::eOne;
			EBlendFactor destAlphaBlendFactor = EBlendFactor::eZero;
			EBlendOp colorBlendOp = EBlendOp::eAdd;
			EBlendOp alphaBlendOp = EBlendOp::eAdd;
		};

		struct ColorAttachmentsBlendStates
		{
			std::array<SingleColorAttachmentBlendStates, 8> attachmentBlendStates = {};
			uint32_t attachmentCount = 0;
		};
#pragma endregion
	public:
		RasterizerStates rasterizationStates = {};
		MultiSampleStates multiSampleStates = {};
		DepthStencilStates depthStencilStates = {};
		ColorAttachmentsBlendStates colorAttachments = {};
	};
}