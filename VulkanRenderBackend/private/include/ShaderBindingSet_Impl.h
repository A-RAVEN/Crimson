#pragma once
#include <RenderInterface/header/ShaderBindingSet.h>
#include <RenderInterface/header/ShaderBindingBuilder.h>
#include "Containers.h"
#include "VulkanApplicationSubobjectBase.h"

namespace graphics_backend
{
	class ShaderBindingSet_Impl : public ShaderBindingSet, public BaseApplicationSubobject
	{
	public:
		ShaderBindingSet_Impl(CVulkanApplication& owner);
		// Í¨¹ý ShaderBindingSet ¼Ì³Ð
		virtual void SetArithmeticValue(std::string const& name, void* pValue) override;
		void Create();
		virtual void DoUpload() override;
		virtual bool UploadingDone() const override;
	private:
		const ShaderBindingBuilder& m_Builder;
		std::vector<uint8_t> m_UploadData;
	};

	class ShaderBindingSetMetadata
	{
	public:
		void Initialize(ShaderBindingBuilder const& builder);
	private:
		std::unordered_map<std::string, std::pair<size_t, size_t>> m_ArithmeticValuePositions;
	};
}