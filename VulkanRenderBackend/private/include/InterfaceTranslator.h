#pragma once
#include<RenderInterface/header/Common.h>
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

	vk::Format ETextureFormatToVkFotmat(ETextureFormat inFormat)
	{
		switch (inFormat)
		{
			case ETextureFormat::E_R8_UNORM:
				return vk::Format::eR8Unorm;
			case ETextureFormat::E_R16_UNORM:
				return vk::Format::eR16Unorm;
			case ETextureFormat::E_R16_SFLOAT:
				return vk::Format::eR16Sfloat;
			case ETextureFormat::E_R8G8_UNORM:
				return vk::Format::eR8G8Unorm;
			case ETextureFormat::E_R16G16_SFLOAT:
				return vk::Format::eR16G16Sfloat;
			case ETextureFormat::E_R8G8B8A8_UNORM:
				return vk::Format::eR8G8B8A8Unorm;
			case ETextureFormat::E_R16G16B16A16_UNORM:
				return vk::Format::eR16G16B16A16Unorm;
			case ETextureFormat::E_R16G16B16A16_SFLOAT:
				return vk::Format::eR16G16B16A16Sfloat;
			case ETextureFormat::E_R32_SFLOAT:
				return vk::Format::eR32Sfloat;
			case ETextureFormat::E_R32G32B32A32_SFLOAT:
				return vk::Format::eR32G32B32A32Sfloat;
			case ETextureFormat::E_INVALID:
				return vk::Format::eUndefined;
		}
	}

	vk::AttachmentLoadOp EAttachmentLoadOpToVkLoadOp(EAttachmentLoadOp inOp)
	{
		switch (inOp)
		{
		case EAttachmentLoadOp::eLoad:
			return vk::AttachmentLoadOp::eLoad;
		case EAttachmentLoadOp::eClear:
			return vk::AttachmentLoadOp::eClear;
		case EAttachmentLoadOp::eDontCare:
			return vk::AttachmentLoadOp::eDontCare;
		}
	}

	vk::AttachmentStoreOp EAttachmentStoreOpToVkStoreOp(EAttachmentStoreOp inOp)
	{
		switch (inOp)
		{
		case EAttachmentStoreOp::eStore:
			return vk::AttachmentStoreOp::eStore;
		case EAttachmentStoreOp::eDontCare:
			return vk::AttachmentStoreOp::eDontCare;
		}
	}

	vk::SampleCountFlagBits EMultiSampleCountToVkSampleCount(EMultiSampleCount sampleCount)
	{
		switch (sampleCount)
		{
		case EMultiSampleCount::e1:
			return vk::SampleCountFlagBits::e1;
			break;
		case EMultiSampleCount::e2:
			return vk::SampleCountFlagBits::e2;
			break;
		case EMultiSampleCount::e4:
			return vk::SampleCountFlagBits::e4;
			break;
		case EMultiSampleCount::e8:
			return vk::SampleCountFlagBits::e8;
			break;
		case EMultiSampleCount::e16:
			return vk::SampleCountFlagBits::e16;
			break;
		case EMultiSampleCount::e32:
			return vk::SampleCountFlagBits::e32;
			break;
		case EMultiSampleCount::e64:
			return vk::SampleCountFlagBits::e64;
			break;
		default:
			return vk::SampleCountFlagBits::e1;
			break;
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
		switch (inStencilOp)
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
		switch (inBlendFactor)
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
		switch (inBlendOp)
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
		if (inColorMask & EColorChannelMask::eR)
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

}