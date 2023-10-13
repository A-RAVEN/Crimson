#pragma once
#include "Common.h"
#include <SharedTools/header/RAII.h>
#include "GPUTexture.h"
#include "TextureSampler.h"
#include "TextureHandle.h"

namespace graphics_backend
{
	class ShaderConstantSet;
	class ShaderBindingSetHandle_Impl
	{
	public:
		ShaderBindingSetHandle_Impl() = default;
		ShaderBindingSetHandle_Impl(ShaderBindingSetHandle_Impl const&) = delete;
		ShaderBindingSetHandle_Impl& operator=(ShaderBindingSetHandle_Impl const&) = delete;
		ShaderBindingSetHandle_Impl(ShaderBindingSetHandle_Impl&&) = default;
		ShaderBindingSetHandle_Impl(TIndex shaderBindingDescIndex) : m_DescIndex(shaderBindingDescIndex) {}
		TIndex GetBuilderIndex() const { return m_DescIndex; }
		void SetConstantSet(std::string const& name, std::shared_ptr<ShaderConstantSet> const& pConstantSet);
		void SetTexture(std::string const& name
			, std::shared_ptr<GPUTexture> const& pTexture);
		void SetSampler(std::string const& name
			, std::shared_ptr<TextureSampler> const& pSampler);
	private:
		TIndex m_DescIndex = INVALID_INDEX;
		//External Constant Sets(i.e. global constants shared between graphs)
		std::unordered_map<std::string, std::shared_ptr<ShaderConstantSet>> m_ExternalConstantSets;
		//External Textures(i.e. resource manager managed textures)
		std::unordered_map<std::string, std::shared_ptr<GPUTexture>> m_ExternalTextures;
		std::unordered_map<std::string, std::shared_ptr<TextureSampler>> m_ExternalSamplers;
		//Internal Managed Or Registered Textures
		std::unordered_map<std::string, TextureHandle> m_InternalTextures;
	};

	using ShaderBindingSetHandle = raii_utils::TRAIIContainer<ShaderBindingSetHandle_Impl>;
}