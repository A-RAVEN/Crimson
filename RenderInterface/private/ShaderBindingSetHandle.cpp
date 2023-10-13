#include "pch.h"
#include "header/ShaderBindingSetHandle.h"

namespace graphics_backend
{
	void ShaderBindingSetHandle_Impl::SetConstantSet(std::string const& name, std::shared_ptr<ShaderConstantSet> const& pConstantSet)
	{
		auto found = m_ExternalConstantSets.find(name);
		if (found != m_ExternalConstantSets.end())
		{
			found->second = pConstantSet;
		}
		else
		{
			m_ExternalConstantSets.insert(std::make_pair(name, pConstantSet));
		}
	}
	void ShaderBindingSetHandle_Impl::SetTexture(std::string const& name, std::shared_ptr<GPUTexture> const& pTexture)
	{
		auto found = m_ExternalTextures.find(name);
		if (found != m_ExternalTextures.end())
		{
			found->second = pTexture;
		}
		else
		{
			m_ExternalTextures.insert(std::make_pair(name, pTexture));
		}
	}
	void ShaderBindingSetHandle_Impl::SetSampler(std::string const& name, std::shared_ptr<TextureSampler> const& pSampler)
	{
		auto found = m_ExternalSamplers.find(name);
		if (found != m_ExternalSamplers.end())
		{
			found->second = pSampler;
		}
		else
		{
			m_ExternalSamplers.insert(std::make_pair(name, pSampler));
		}
	}
}

