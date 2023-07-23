#pragma once
#include <cstdint>
#include "TEnumCommon.h"
#include <array>

#include "Common.h"
#include "HashCommon.h"
#include <SharedTools/header/uhash.h>
using namespace hash_utils;

struct RasterizerStates
{
public:
	bool enableDepthClamp = false;
	bool discardRasterization = false;
	ECullMode cullMode = ECullMode::eBack;
	EFrontFace frontFace = EFrontFace::eClockWise;
	EPolygonMode polygonMode = EPolygonMode::eFill;
	float lineWidth = 1.0f;

	bool operator==(RasterizerStates const& rhs) const
	{
		return enableDepthClamp == rhs.enableDepthClamp
			&& discardRasterization == rhs.discardRasterization
			&& cullMode == rhs.cullMode
			&& frontFace == rhs.frontFace
			&& polygonMode == rhs.polygonMode
			&& lineWidth == rhs.lineWidth;
	}
};
template<>
struct is_contiguously_hashable<RasterizerStates> : public std::true_type {};

struct MultiSampleStates
{
	EMultiSampleCount msCount = EMultiSampleCount::e1;

bool operator==(MultiSampleStates const& rhs) const
	{
		return msCount == rhs.msCount;
	}
};

template<>
struct is_contiguously_hashable<MultiSampleStates> : public std::true_type {};

struct DepthStencilStates
{
	struct StencilStates
	{
		EStencilOp failOp = EStencilOp::eKeep;
		EStencilOp passOp = EStencilOp::eKeep;
		EStencilOp depthFailOp = EStencilOp::eKeep;
		ECompareOp compareOp = ECompareOp::eAlways;
		uint32_t compareMask = 0u;
		uint32_t writeMask = 0u;
		uint32_t reference = 0u;

		bool operator==(StencilStates const& rhs) const
		{
			return failOp == rhs.failOp
				&& passOp == rhs.passOp
				&& depthFailOp == rhs.depthFailOp
				&& compareOp == rhs.compareOp
				&& compareMask == rhs.compareMask
				&& writeMask == rhs.writeMask
				&& reference == rhs.reference;
		}
	};

	bool depthTestEnable = false;
	bool depthWriteEnable = false;
	ECompareOp depthCompareOp = ECompareOp::eGEqual;

	bool stencilTestEnable = false;
	StencilStates stencilStateFront = {};
	StencilStates stencilStateBack = {};

	bool operator==(DepthStencilStates const& rhs) const
	{
		return depthTestEnable == rhs.depthTestEnable
			&& depthWriteEnable == rhs.depthWriteEnable
			&& depthCompareOp == rhs.depthCompareOp
			&& stencilTestEnable == rhs.stencilTestEnable
			&& stencilStateFront == rhs.stencilStateFront
			&& stencilStateBack == rhs.stencilStateBack;
	}
};

template<>
struct is_contiguously_hashable<DepthStencilStates> : public std::true_type {};

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

	bool operator==(SingleColorAttachmentBlendStates const& rhs) const
	{
		return blendEnable == rhs.blendEnable
			&& channelMask == rhs.channelMask
			&& sourceColorBlendFactor == rhs.sourceColorBlendFactor
			&& destColorBlendFactor == rhs.destColorBlendFactor
			&& sourceAlphaBlendFactor == rhs.sourceAlphaBlendFactor
			&& destAlphaBlendFactor == rhs.destAlphaBlendFactor
			&& colorBlendOp == rhs.colorBlendOp
			&& alphaBlendOp == rhs.alphaBlendOp;
	}
};

template<>
struct is_contiguously_hashable<SingleColorAttachmentBlendStates> : public std::true_type {};

struct ColorAttachmentsBlendStates
{
	std::array<SingleColorAttachmentBlendStates, 8> attachmentBlendStates = {};
	uint32_t attachmentCount = 0;

	bool operator==(ColorAttachmentsBlendStates const& rhs) const
	{
		if (attachmentCount != rhs.attachmentCount)
		{
			return false;
		}
		for (uint32_t i = 0; i < attachmentCount; ++i)
		{
			if (!(attachmentBlendStates[i] == rhs.attachmentBlendStates[i]))
			{
				return false;
			}
		}
		return true;
	}
};
template<>
struct is_contiguously_hashable<ColorAttachmentsBlendStates> : public std::true_type {};
/// <summary>
/// Pipeline States That May Share During One Pass
/// Pure Data Struct, Not The Real One Used In Backend API
/// </summary>
class CPipelineStateObject
{
public:
	RasterizerStates rasterizationStates = {};
	MultiSampleStates multiSampleStates = {};
	DepthStencilStates depthStencilStates = {};
	ColorAttachmentsBlendStates colorAttachments = {};

	bool operator==(CPipelineStateObject const& rhs) const
	{
		return rasterizationStates == rhs.rasterizationStates
			&& multiSampleStates == rhs.multiSampleStates
			&& depthStencilStates == rhs.depthStencilStates
			&& colorAttachments == rhs.colorAttachments;
	}
};

template<>
struct is_contiguously_hashable<CPipelineStateObject> : public std::true_type {};
