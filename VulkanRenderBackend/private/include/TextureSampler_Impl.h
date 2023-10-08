#pragma once
#include <RenderInterface/header/TextureSampler.h>
#include "HashPool.h"
#include "VulkanApplicationSubobjectBase.h"

namespace graphics_backend
{
	class TextureSampler_Impl : public TextureSampler, public BaseApplicationSubobject
	{
	public:
		TextureSampler_Impl(CVulkanApplication& app);
		virtual TextureSamplerDescriptor const& GetDescriptor() const override { return *p_Descriptor; }
		void Create(TextureSamplerDescriptor const& descriptor);
		void Initialize(TextureSamplerDescriptor const& descriptor);
		virtual void Release() override;
	private:
		TextureSamplerDescriptor const* p_Descriptor;
		vk::Sampler m_Sampler;
	};

	using TextureSamplerObjectDic = HashPool<TextureSamplerDescriptor, TextureSampler_Impl>;
}


