#pragma once
#include "Common.h"
#include "TextureSampler.h"
#include "GPUTexture.h"

namespace graphics_backend
{
	class ShaderConstantSet
	{
	public:
		template<typename T>
		void SetValue(std::string const& name, T const& value)
		{
			SetValue(name, (void*)&value);
		}
		virtual void SetValue(std::string const& name, void* pValue) = 0;
		virtual void UploadAsync() = 0;
		virtual bool UploadingDone() const = 0;
		virtual std::string const& GetName() const = 0;
	};

	class ShaderBindingSet
	{
	public:
		virtual void SetConstantSet(std::string const& name, std::shared_ptr<ShaderConstantSet> const& pConstantSet) = 0;
		virtual void SetTexture(std::string const& name
			, std::shared_ptr<GPUTexture> const& pTexture) = 0;
		virtual void SetSampler(std::string const& name
			, std::shared_ptr<TextureSampler> const& pSampler) = 0;
		virtual void UploadAsync() = 0;
		virtual bool UploadingDone() const = 0;
	};
}