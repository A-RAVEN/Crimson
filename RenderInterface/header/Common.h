#pragma once
#include <SharedTools/header/DebugUtils.h>
#include <cstdint>
#include <SharedTools/header/uenum.h>

using TIndex = uint32_t;
constexpr TIndex INVALID_INDEX = std::numeric_limits<TIndex>::max();

enum class ECompileShaderType : uint8_t
{
	eVert = 0,
	eTessCtr,
	eTessEvl,
	eGeom,
	eFrag,
	eComp,
	//nvidia mesh shader
	eTask,
	eMesh,
	//nvidia ray tracing shader
	eRaygen,
	eAnyhit,
	eClosehit,
	eMiss,
	eIntersect,
	eCallable,
	eMax,
};

enum class EShaderTextureType : uint8_t
{
	eTexture1D = 0,
	eRWTexture1D,
	eTexture2D,
	eRWTexture2D,
	eTexture3D,
	eRWTexture3D,
	eTexelBuffer,
	eRWTexelBuffer,
	eTextureCube,
	eSubpassInput,
};

enum class EShaderBufferType : uint8_t
{
	eConstantBuffer = 0,
	eStructuredBuffer,
	eRWStructuredBuffer,
};

enum class EShaderSamplerType : uint8_t
{
	eSampler = 0,
	//eSamplerComparison,
};

enum class EShaderSourceType : uint8_t
{
	eHLSL = 0,
	eGLSL,
};

enum class EIndexBufferType : uint8_t
{
	e16,
	e32
};

enum class ETextureType : uint8_t
{
	e1D,
	e2D,
	e3D,
	e2DArray,
	eCubeMap,
};

enum class ETextureFormat : uint8_t
{
	//标记浮点值
	E_FLOAT_TYPE_CATEGORY_BEGIN,
	E_R8_UNORM,
	E_R16_UNORM,
	E_R16_SFLOAT,

	E_R8G8_UNORM,
	E_R16G16_SFLOAT,

	E_R8G8B8A8_UNORM,
	E_R16G16B16A16_UNORM,
	E_R16G16B16A16_SFLOAT,

	E_B8G8R8A8_UNORM,

	E_R32_SFLOAT,
	E_R32G32B32A32_SFLOAT,
	E_FLOAT_TYPE_CATEGORY_END,

	//标记整形值
	E_INT_TYPE_CATEGORY_BEGIN,
	E_INT_TYPE_CATEGORY_END,

	//标记无符号整形值
	E_UINT_TYPE_CATEGORY_BEGIN,
	E_UINT_TYPE_CATEGORY_END,

	//标记深度，模板值
	E_DEPTHSTENCIL_TYPE_CATEGORY_BEGIN,
	//仅深度
	E_DEPTHONLY_TYPE_CATEGORY_BEGIN,
	E_D32_SFLOAT,
	E_DEPTHONLY_TYPE_CATEGORY_END,
	E_D32_SFLOAT_S8_UINT,
	E_DEPTHSTENCIL_TYPE_CATEGORY_END,

	E_INVALID,
};

constexpr bool IsDepthOnlyFormat(ETextureFormat format)
{
	return (format > ETextureFormat::E_DEPTHONLY_TYPE_CATEGORY_BEGIN && format < ETextureFormat::E_DEPTHONLY_TYPE_CATEGORY_END);
}

constexpr bool IsDepthStencilFormat(ETextureFormat format)
{
	return (format > ETextureFormat::E_DEPTHSTENCIL_TYPE_CATEGORY_BEGIN && format < ETextureFormat::E_DEPTHSTENCIL_TYPE_CATEGORY_END);
}

constexpr bool IsFloatFormat(ETextureFormat format)
{
	return (format > ETextureFormat::E_FLOAT_TYPE_CATEGORY_BEGIN && format < ETextureFormat::E_FLOAT_TYPE_CATEGORY_END);
}

constexpr bool IsIntFormat(ETextureFormat format)
{
	return (format > ETextureFormat::E_INT_TYPE_CATEGORY_BEGIN && format < ETextureFormat::E_INT_TYPE_CATEGORY_END);
}

constexpr bool IsUintFormat(ETextureFormat format)
{
	return (format > ETextureFormat::E_UINT_TYPE_CATEGORY_BEGIN && format < ETextureFormat::E_UINT_TYPE_CATEGORY_END);
}

enum class EBufferUsage : uint16_t
{
	eConstantBuffer = 1,
	eStructuredBuffer = 1 << 1,
	eVertexBuffer = 1 << 2,
	eIndexBuffer = 1 << 3,
	eDataSrc = 1 << 4,
	eDataDst = 1 << 5,
	eMaxBit = 5
};

template <>
struct uenum::TEnumTraits<EBufferUsage>
{
	static constexpr bool is_bitmask = true;
};

using EBufferUsageFlags = uenum::EnumFlags<EBufferUsage>;


enum class ETextureAccessType : uint8_t
{
	eSampled = 1 << 0,
	eRT = 1 << 1,
	eSubpassInput = 1 << 2,
	eUnorderedAccess = 1 << 3,
	eTransferSrc = 1 << 4,
	eTransferDst = 1 << 5,
	eAccessType_Max = 5,
};

template <>
struct uenum::TEnumTraits<ETextureAccessType>
{
	static constexpr bool is_bitmask = true;
};


using ETextureAccessTypeFlags = uenum::EnumFlags<ETextureAccessType>;

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

using EColorChannelMaskFlags = uenum::EnumFlags<EColorChannelMask>;

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

union GraphicsClearValue
{
public:
	struct ClearColorValue
	{
		float r;
		float g;
		float b;
		float a;
	} color;
	struct ClearDepthStencilValue
	{
		float depth;
		uint32_t stencil;
	} depthStencil;

	static constexpr GraphicsClearValue ClearColor(float r, float g, float b, float a)
	{
		GraphicsClearValue result{};
		result.color = ClearColorValue{ r, g, b, a };
		return result;
	}

	static constexpr GraphicsClearValue ClearDepthStencil(float depth, uint32_t stencil)
	{
		GraphicsClearValue result{};
		result.depthStencil = ClearDepthStencilValue{ depth, stencil };
		return result;
	}

	bool operator==(GraphicsClearValue const& rhs) const
	{
		return (std::memcmp(this, &rhs, sizeof(GraphicsClearValue)) == 0);
	}
};

template<>
struct hash_utils::is_contiguously_hashable<GraphicsClearValue> : public std::true_type {};