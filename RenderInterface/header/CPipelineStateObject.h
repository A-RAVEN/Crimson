#pragma once
#include <cstdint>
#include "TEnumCommon.h"
#include <array>

#include "Common.h"
#include "HashCommon.h"

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

template<> struct std::hash<graphics_backend::CPipelineStateObject::ColorAttachmentsBlendStates>
{
	std::size_t operator()(graphics_backend::CPipelineStateObject::ColorAttachmentsBlendStates const& source) const noexcept
	{
		auto h = std::hash<uint32_t>{}(source.attachmentCount);
		for(uint32_t i = 0; i < source.attachmentCount; ++i)
		{
			hash_combine(h, source.attachmentBlendStates[i]);
		}
		return h;
	}
};