#pragma once
#include <SharedTools/header/uhash.h>

namespace graphics_backend
{
	enum class ETextureSamplerFilterMode
	{
		eNearest,
		eLinear,
	};

	enum class ETextureSamplerAddressMode
	{
		eRepeat,
		eMirroredRepeat,
		eClampToEdge,
		eClampToBorder,
		//eMirrorClampToEdge,
	};

	enum class ETextureSamplerBorderColor
	{
		eTransparentBlack,
		eOpaqueBlack,
		eOpaqueWhite,
	};

	struct TextureSamplerDescriptor
	{
	public:
		ETextureSamplerFilterMode magFilterMode = ETextureSamplerFilterMode::eLinear;
		ETextureSamplerFilterMode minFilterMode = ETextureSamplerFilterMode::eLinear;
		ETextureSamplerFilterMode mipmapFilterMode = ETextureSamplerFilterMode::eLinear;
		ETextureSamplerAddressMode addressModeU = ETextureSamplerAddressMode::eClampToEdge;
		ETextureSamplerAddressMode addressModeV = ETextureSamplerAddressMode::eClampToEdge;
		ETextureSamplerAddressMode addressModeW = ETextureSamplerAddressMode::eClampToEdge;
		ETextureSamplerBorderColor boarderColor = ETextureSamplerBorderColor::eTransparentBlack;
		bool integerFormat = false;

		bool operator==(TextureSamplerDescriptor const& other) const noexcept
		{
			return magFilterMode == other.magFilterMode
				&& minFilterMode == other.minFilterMode
				&& mipmapFilterMode == other.mipmapFilterMode
				&& addressModeU == other.addressModeU
				&& addressModeV == other.addressModeV
				&& addressModeW == other.addressModeW
				&& boarderColor == other.boarderColor
				&& integerFormat == other.integerFormat;
		}
	};

	class TextureSampler
	{
	public:
		virtual TextureSamplerDescriptor const& GetDescriptor() const = 0;
	};
}

template<>
struct hash_utils::is_contiguously_hashable<graphics_backend::TextureSamplerDescriptor> : public std::true_type {};