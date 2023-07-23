#pragma once
#include "VulkanApplicationSubobjectBase.h"
#include "RenderInterface/header/CShaderModule.h"
#include <SharedTools/header/uhash.h>
#include <unordered_map>
#include <private/include/HashPool.h>

namespace graphics_backend
{
	struct ShaderModuleDescritor
	{
		std::shared_ptr<const ShaderProvider> provider;

		bool operator==(ShaderModuleDescritor const& other) const
		{
			if(provider == nullptr || other.provider == nullptr)
				return false;
			else if(provider == other.provider)
				return true;
			else if(provider->GetUniqueName() == other.provider->GetUniqueName())
				return true;
		}

		template <class HashAlgorithm>
		friend void hash_append(HashAlgorithm& h, ShaderModuleDescritor const& shadermodule_desc) noexcept
		{
			hash_append(h, shadermodule_desc.provider.get());
		}
	};

	class CShaderModuleObject : public BaseApplicationSubobject
	{
	public:
		CShaderModuleObject(CVulkanApplication& application);
		void Create(ShaderModuleDescritor const& descriptor);
		virtual void Release() override;
		vk::ShaderModule GetShaderModule() const { return m_ShaderModule; }
	private:
		vk::ShaderModule m_ShaderModule = nullptr;
	};

	using ShaderModuleObjectDic = HashPool<ShaderModuleDescritor, CShaderModuleObject>;
}
