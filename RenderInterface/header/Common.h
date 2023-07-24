#pragma once
#include <cstdint>
#include <SharedTools/header/uenum.h>

#ifdef VULKANRENDERBACKEND_EXPORTS
#define ANY_RENDER_BACKEND_EXPORTS 1
#endif

#ifdef ANY_RENDER_BACKEND_EXPORTS
#define RENDERBACKEND_API __declspec(dllexport)
#else
#define RENDERBACKEND_API
#endif

#ifdef RENDERINTERFACE_EXPORTS
#define RENDERINTERFACE_API __declspec(dllexport)
#else
#define RENDERINTERFACE_API
#endif

using TIndex = uint32_t;
constexpr TIndex INVALID_INDEX = std::numeric_limits<TIndex>::max();

enum class ETextureFormat : uint8_t
{
	E_R8_UNORM,
	E_R16_UNORM,
	E_R16_SFLOAT,

	E_R8G8_UNORM,
	E_R16G16_SFLOAT,

	E_R8G8B8A8_UNORM,
	E_R16G16B16A16_UNORM,
	E_R16G16B16A16_SFLOAT,

	E_R32_SFLOAT,
	E_R32G32B32A32_SFLOAT,
	E_INVALID,
};

enum class CTextureAccessType
{
	E_ReadOnly,
	E_RT,
	E_UAV,
	E_AccessType_Invalid,
};

class CTextureInfo
{
	uint32_t m_Width = 0;
	uint32_t m_Height = 0;
	uint32_t m_Slices = 0;
	uint32_t m_MipCount = 0;
	ETextureFormat m_TextureFormat = ETextureFormat::E_INVALID;
	CTextureAccessType m_AccessType = CTextureAccessType::E_AccessType_Invalid;
};


enum class EAttachmentLoadOp
{
	eLoad = 0,
	eClear,
	eDontCare,
};

enum class EAttachmentStoreOp
{
	eStore = 0,
	eDontCare,
};

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

template <>
struct uenum::TEnumTraits<EColorChannelMask>
{
	static constexpr bool is_bitmask = true;
};

using EColorChannelMaskFlags = uenum::EnumFlags<EColorChannelMask>; //std::underlying_type_t<EColorChannelMask>;

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

enum class ETopology
{
	eTriangleList = 0,
	eTriangleStrip,
	ePointList,
	eLineList,
	eLineStrip,
};

enum class VertexInputFormat
{
	eR32_SFloat = 0,
	eR32G32_SFloat,
	eR32G32B32_SFloat,
	eR32G32B32A32_SFloat,
	eR32_UInt,
	eR32_SInt,
};