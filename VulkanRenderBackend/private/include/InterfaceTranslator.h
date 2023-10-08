#pragma once
#include<RenderInterface/header/Common.h>
#include<RenderInterface/header/TextureSampler.h>
#include <RenderInterface/header/GPUTexture.h>

namespace graphics_backend
{
	struct VulkanImageInfo
	{
	public:
		vk::ImageCreateFlags createFlags;
		vk::ImageType imageType;
		vk::ImageViewType defaultImageViewType;
	};

	constexpr vk::Format VertexInputFormatToVkFormat(VertexInputFormat inFormat)
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

	constexpr vk::Format ETextureFormatToVkFotmat(ETextureFormat inFormat)
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
			case ETextureFormat::E_B8G8R8A8_UNORM:
				return vk::Format::eB8G8R8A8Unorm;
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
			case ETextureFormat::E_D32_SFLOAT:
				return vk::Format::eD32Sfloat;
			case ETextureFormat::E_D32_SFLOAT_S8_UINT:
				return vk::Format::eD32SfloatS8Uint;
			case ETextureFormat::E_INVALID:
			default:
				return vk::Format::eUndefined;
		}
	}



	constexpr vk::BufferImageCopy GPUTextureDescriptorToBufferImageCopy(GPUTextureDescriptor const& descriptor)
	{
		bool isDepthOnly = IsDepthOnlyFormat(descriptor.format);
		bool isDepthStencil = IsDepthStencilFormat(descriptor.format);
		bool is3D = descriptor.textureType == ETextureType::e3D;
		vk::BufferImageCopy result{};
		if (isDepthOnly)
		{
			result.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eDepth;
		}
		else if (isDepthStencil)
		{
			result.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eDepth
				| vk::ImageAspectFlagBits::eStencil;
		}
		else
		{
			result.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		}
		result.imageSubresource.mipLevel = 0;
		result.imageSubresource.baseArrayLayer = 0;
		result.imageSubresource.layerCount = is3D ? 1 : descriptor.layers;
		result.bufferImageHeight = 0;
		result.bufferOffset = 0;
		result.imageOffset = vk::Offset3D{ 0, 0, 0 };
		result.imageExtent = vk::Extent3D(descriptor.width, descriptor.height, is3D ? descriptor.layers : 1);
		return result;
	}

	constexpr vk::ImageUsageFlags ETextureAccessTypeToVulkanImageUsageFlags(ETextureFormat format, ETextureAccessTypeFlags accessType)
	{
		vk::ImageUsageFlags resultUsage{};
		for (std::underlying_type_t<ETextureAccessType> accessTypeId = 0
			; accessTypeId <= static_cast<std::underlying_type_t<ETextureAccessType>>(ETextureAccessType::eAccessType_Max)
			; ++accessTypeId)
		{
			ETextureAccessType typemask = static_cast<ETextureAccessType>(1 << accessTypeId);
			if (accessType & typemask)
			{
				switch (typemask)
				{
				case ETextureAccessType::eSampled:
					resultUsage |= vk::ImageUsageFlagBits::eSampled;
					break;
				case ETextureAccessType::eRT:
					if (IsDepthStencilFormat(format))
					{
						resultUsage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
					}
					else
					{
						resultUsage |= vk::ImageUsageFlagBits::eColorAttachment;
					}
					break;
				case ETextureAccessType::eSubpassInput:
					resultUsage |= vk::ImageUsageFlagBits::eInputAttachment;
					break;
				case ETextureAccessType::eUnorderedAccess:
					resultUsage |= vk::ImageUsageFlagBits::eStorage;
					break;
				case ETextureAccessType::eTransferDst:
					resultUsage |= vk::ImageUsageFlagBits::eTransferDst;
					break;
				case ETextureAccessType::eTransferSrc:
					resultUsage |= vk::ImageUsageFlagBits::eTransferSrc;
					break;
				}
			}
		}
		return resultUsage;
	}

	constexpr VulkanImageInfo ETextureTypeToVulkanImageInfo(ETextureType textureType)
	{
		switch (textureType)
		{
		case ETextureType::e1D:
			return VulkanImageInfo{ {}, vk::ImageType::e1D, vk::ImageViewType::e1D };
		case ETextureType::e2D:
			return VulkanImageInfo{ {}, vk::ImageType::e2D , vk::ImageViewType::e2D };
		case ETextureType::e3D:
			return VulkanImageInfo{ {}, vk::ImageType::e3D , vk::ImageViewType::e3D };
		case ETextureType::e2DArray:
			return VulkanImageInfo{ vk::ImageCreateFlagBits::e2DArrayCompatible, vk::ImageType::e2D, vk::ImageViewType::e2DArray };
		case ETextureType::eCubeMap:
			return VulkanImageInfo{ vk::ImageCreateFlagBits::eCubeCompatible, vk::ImageType::e2D, vk::ImageViewType::eCube };
		}
		CA_LOG_ERR("Unknown Texture Type!");
		return {};
	}



	constexpr ETextureFormat VkFotmatToETextureFormat(vk::Format inFormat)
	{
		switch (inFormat)
		{
		case vk::Format::eR8Unorm:
			return ETextureFormat::E_R8_UNORM;
		case vk::Format::eR16Unorm:
			return ETextureFormat::E_R16_UNORM;
		case vk::Format::eR16Sfloat:
			return ETextureFormat::E_R16_SFLOAT;
		case vk::Format::eR8G8Unorm:
			return ETextureFormat::E_R8G8_UNORM;
		case vk::Format::eB8G8R8A8Unorm:
			return ETextureFormat::E_B8G8R8A8_UNORM;
		case vk::Format::eR16G16Sfloat:
			return ETextureFormat::E_R16G16_SFLOAT;
		case vk::Format::eR8G8B8A8Unorm:
			return ETextureFormat::E_R8G8B8A8_UNORM;
		case vk::Format::eR16G16B16A16Unorm:
			return ETextureFormat::E_R16G16B16A16_UNORM;
		case vk::Format::eR16G16B16A16Sfloat:
			return ETextureFormat::E_R16G16B16A16_SFLOAT;
		case vk::Format::eR32Sfloat:
			return ETextureFormat::E_R32_SFLOAT;
		case vk::Format::eR32G32B32A32Sfloat:
			return ETextureFormat::E_R32G32B32A32_SFLOAT;
		case vk::Format::eD32Sfloat:
			return ETextureFormat::E_D32_SFLOAT;
		case vk::Format::eD32SfloatS8Uint:
			return ETextureFormat::E_D32_SFLOAT_S8_UINT;
		default:
			return ETextureFormat::E_INVALID;
		}
	}

	constexpr vk::Filter ETextureSamplerFilterModeToVkFilter(ETextureSamplerFilterMode inFilterMode)
	{
		switch (inFilterMode)
		{
		case ETextureSamplerFilterMode::eNearest:
			return vk::Filter::eNearest;
		case ETextureSamplerFilterMode::eLinear:
			return vk::Filter::eLinear;
		default:
			return vk::Filter::eLinear;
		}
	}

	constexpr vk::SamplerMipmapMode ETextureSamplerFilterModeToVkMipmapMode(ETextureSamplerFilterMode inFilterMode)
	{
		switch (inFilterMode)
		{
		case ETextureSamplerFilterMode::eNearest:
			return vk::SamplerMipmapMode::eNearest;
		case ETextureSamplerFilterMode::eLinear:
			return vk::SamplerMipmapMode::eLinear;
		default:
			return vk::SamplerMipmapMode::eLinear;
		}
	}

	constexpr vk::SamplerAddressMode ETextureSamplerAddressModeToVkSamplerAddressMode(ETextureSamplerAddressMode inAddressMode)
	{
		switch (inAddressMode)
		{
			case ETextureSamplerAddressMode::eRepeat:
				return vk::SamplerAddressMode::eRepeat;
			case ETextureSamplerAddressMode::eMirroredRepeat:
				return vk::SamplerAddressMode::eMirroredRepeat;
			case ETextureSamplerAddressMode::eClampToEdge:
				return vk::SamplerAddressMode::eClampToEdge;
			case ETextureSamplerAddressMode::eClampToBorder:
				return vk::SamplerAddressMode::eClampToBorder;
		}
		return vk::SamplerAddressMode::eClampToEdge;
	}

	constexpr vk::BorderColor ETextureSamplerBorderColorToVkBorderColor(ETextureSamplerBorderColor inBorderColor
	, bool integerFormat)
	{
		switch (inBorderColor)
		{
		case ETextureSamplerBorderColor::eTransparentBlack:
			return integerFormat ? vk::BorderColor::eIntTransparentBlack : vk::BorderColor::eFloatTransparentBlack;
		case ETextureSamplerBorderColor::eOpaqueBlack:
			return integerFormat ? vk::BorderColor::eIntOpaqueBlack : vk::BorderColor::eFloatOpaqueBlack;
		case ETextureSamplerBorderColor::eOpaqueWhite:
			return integerFormat ? vk::BorderColor::eIntOpaqueWhite : vk::BorderColor::eFloatOpaqueWhite;
		default:
			return vk::BorderColor::eFloatOpaqueWhite;
		}
	}

	constexpr vk::AttachmentLoadOp EAttachmentLoadOpToVkLoadOp(EAttachmentLoadOp inOp)
	{
		switch (inOp)
		{
		case EAttachmentLoadOp::eLoad:
			return vk::AttachmentLoadOp::eLoad;
		case EAttachmentLoadOp::eClear:
			return vk::AttachmentLoadOp::eClear;
		case EAttachmentLoadOp::eDontCare:
		default:
			return vk::AttachmentLoadOp::eDontCare;
		}
	}

	constexpr vk::AttachmentStoreOp EAttachmentStoreOpToVkStoreOp(EAttachmentStoreOp inOp)
	{
		switch (inOp)
		{
		case EAttachmentStoreOp::eStore:
			return vk::AttachmentStoreOp::eStore;
		case EAttachmentStoreOp::eDontCare:
		default:
			return vk::AttachmentStoreOp::eDontCare;
		}
	}

	constexpr vk::SampleCountFlagBits EMultiSampleCountToVkSampleCount(EMultiSampleCount sampleCount)
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

	constexpr vk::PrimitiveTopology ETopologyToVkTopology(ETopology inTopology)
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

	constexpr vk::PolygonMode EPolygonModeTranslate(EPolygonMode inPolygonMode)
	{
		switch (inPolygonMode)
		{
		case EPolygonMode::eFill: return vk::PolygonMode::eFill;
		case EPolygonMode::eLine: return vk::PolygonMode::eLine;
		case EPolygonMode::ePoint: return vk::PolygonMode::ePoint;
		default: return vk::PolygonMode::eFill;
		}
	}

	constexpr vk::CullModeFlags ECullModeTranslate(ECullMode inCullMode)
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

	constexpr vk::FrontFace EFrontFaceTranslate(EFrontFace inFrontFace)
	{
		switch (inFrontFace)
		{
		case EFrontFace::eClockWise: return vk::FrontFace::eClockwise;
		case EFrontFace::eCounterClockWise: return vk::FrontFace::eCounterClockwise;
		default: return vk::FrontFace::eClockwise;
		}
	}

	constexpr vk::StencilOp EStencilOpTranslate(EStencilOp inStencilOp)
	{
		switch (inStencilOp)
		{
		case EStencilOp::eKeep: return vk::StencilOp::eKeep;
		case EStencilOp::eReplace: return vk::StencilOp::eReplace;
		case EStencilOp::eZero: return vk::StencilOp::eZero;
		default: return vk::StencilOp::eKeep;
		}
	}

	constexpr vk::CompareOp ECompareOpTranslate(ECompareOp inCompareOp)
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

	constexpr vk::BlendFactor EBlendFactorTranslate(EBlendFactor inBlendFactor)
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

	constexpr vk::BlendOp EBlendOpTranslate(EBlendOp inBlendOp)
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

	constexpr vk::ColorComponentFlags EColorChannelMaskTranslate(EColorChannelMaskFlags inColorMask)
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

	constexpr vk::SampleCountFlagBits ESampleCountTranslate(EMultiSampleCount inSampleCount)
	{
		switch (inSampleCount)
		{
		case EMultiSampleCount::e1: return vk::SampleCountFlagBits::e1;
		case EMultiSampleCount::e2: return vk::SampleCountFlagBits::e2;
		case EMultiSampleCount::e4: return vk::SampleCountFlagBits::e4;
		case EMultiSampleCount::e8: return vk::SampleCountFlagBits::e8;
		case EMultiSampleCount::e16: return vk::SampleCountFlagBits::e16;
		case EMultiSampleCount::e32: return vk::SampleCountFlagBits::e32;
		case EMultiSampleCount::e64: return vk::SampleCountFlagBits::e64;
		default: return vk::SampleCountFlagBits::e1;
		}
	}

	constexpr vk::BufferUsageFlagBits EBufferUsageTranslate(EBufferUsage inUsage)
	{
		switch (inUsage)
		{
		case EBufferUsage::eConstantBuffer:
			return vk::BufferUsageFlagBits::eUniformBuffer;
		case EBufferUsage::eStructuredBuffer:
			return vk::BufferUsageFlagBits::eStorageBuffer;
		case EBufferUsage::eVertexBuffer:
			return vk::BufferUsageFlagBits::eVertexBuffer;
		case EBufferUsage::eIndexBuffer:
			return vk::BufferUsageFlagBits::eIndexBuffer;
		case EBufferUsage::eDataDst:
			return vk::BufferUsageFlagBits::eTransferDst;
		case EBufferUsage::eDataSrc:
			return vk::BufferUsageFlagBits::eTransferSrc;
		default: return vk::BufferUsageFlagBits::eUniformBuffer;
		}
	}

	static vk::BufferUsageFlags EBufferUsageFlagsTranslate(EBufferUsageFlags inUsageFlags)
	{
		vk::BufferUsageFlags result;
		for (uint32_t i = 0
			; i <= static_cast<uint32_t>(EBufferUsage::eMaxBit)
			; ++i)
		{
			EBufferUsage itrUsage = static_cast<EBufferUsage>(1 << i);
			if (inUsageFlags & itrUsage)
			{
				result |= EBufferUsageTranslate(itrUsage);
			}
		}
		return result;
	}

	constexpr vk::IndexType EIndexBufferTypeTranslate(EIndexBufferType inIndexType)
	{
		switch (inIndexType)
		{
		case EIndexBufferType::e16:
			return vk::IndexType::eUint16;
		case EIndexBufferType::e32:
			return vk::IndexType::eUint32;
		default: return vk::IndexType::eUint32;
		}
	}

	constexpr vk::ClearValue AttachmentClearValueTranslate(GraphicsClearValue const& inClearValue, ETextureFormat inFormat)
	{
		vk::ClearValue result{};
		//标记浮点值
		if (IsFloatFormat(inFormat))
		{
			result.color = vk::ClearColorValue(std::array<float, 4>{
					inClearValue.color.r
					, inClearValue.color.g
					, inClearValue.color.b
					, inClearValue.color.a
				});
		}
		//标记整形值
		else if (IsIntFormat(inFormat))
		{

		}
		//标记无符号整形值
		else if (IsUintFormat(inFormat))
		{

		}
		//标记深度，模板值
		else if (IsDepthStencilFormat(inFormat))
		{

		}
		return result;
	}
}