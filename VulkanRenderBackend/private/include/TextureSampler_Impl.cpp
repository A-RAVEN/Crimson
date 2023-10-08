#include "private/include/pch.h"
#include "TextureSampler_Impl.h"
#include "InterfaceTranslator.h"

namespace graphics_backend
{
	TextureSampler_Impl::TextureSampler_Impl(CVulkanApplication& app) : BaseApplicationSubobject(app)
	{
	}

	void TextureSampler_Impl::Create(TextureSamplerDescriptor const& descriptor)
	{
		Initialize(descriptor);
	}

	void TextureSampler_Impl::Initialize(TextureSamplerDescriptor const& descriptor)
	{
		p_Descriptor = &descriptor;
		vk::SamplerCreateInfo samplerCreateInfo{
			{}
			, ETextureSamplerFilterModeToVkFilter(descriptor.magFilterMode)
				, ETextureSamplerFilterModeToVkFilter(descriptor.minFilterMode)
				, ETextureSamplerFilterModeToVkMipmapMode(descriptor.mipmapFilterMode)
				, ETextureSamplerAddressModeToVkSamplerAddressMode(descriptor.addressModeU)
				, ETextureSamplerAddressModeToVkSamplerAddressMode(descriptor.addressModeV)
				, ETextureSamplerAddressModeToVkSamplerAddressMode(descriptor.addressModeW)
				, 0.0f
				, vk::Bool32(false)
				, 0.0f
				, vk::Bool32(false)
				, vk::CompareOp::eNever
				, 0.0f
				, 0.0f
				, ETextureSamplerBorderColorToVkBorderColor(descriptor.boarderColor, descriptor.integerFormat)
				, vk::Bool32(false)
		};
		m_Sampler = GetDevice().createSampler(samplerCreateInfo);
	}
	void TextureSampler_Impl::Release()
	{
		GetDevice().destroySampler(m_Sampler);
		p_Descriptor = nullptr;
		m_Sampler = nullptr;
	}
}