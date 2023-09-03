#pragma once
#include <RenderInterface/header/ShaderBindingSet.h>
#include <RenderInterface/header/ShaderBindingBuilder.h>
#include "Containers.h"
#include "VulkanApplicationSubobjectBase.h"

namespace graphics_backend
{
	class ShaderBindingSetMetadata;

	class ShaderBindingSet_Impl : public ShaderBindingSet, public BaseApplicationSubobject
	{
	public:
		ShaderBindingSet_Impl(CVulkanApplication& owner);
		// 通过 ShaderBindingSet 继承
		virtual void SetArithmeticValue(std::string const& name, void* pValue) override;
		void Initialize(ShaderBindingSetMetadata const* inMetaData);
		virtual void DoUpload() override;
		virtual bool UploadingDone() const override;
	private:
		ShaderBindingSetMetadata const* p_Metadata;
		std::vector<uint8_t> m_UploadData;
	};

	class ShaderBindingSetMetadata
	{
	public:
		void Initialize(ShaderBindingBuilder const& builder);
		std::unordered_map<std::string, std::pair<size_t, size_t>> const& GetArithmeticValuePositions() const;
		size_t GetTotalSize() const;
		std::string const& GetSetSpaceName() const;
	private:
		ShaderBindingBuilder const* p_Builder;
		size_t m_TotalSize = 0;
		std::unordered_map<std::string, std::pair<size_t, size_t>> m_ArithmeticValuePositions;
	};

	class ShaderBindingSetAllocator : public BaseApplicationSubobject
	{
	public:
		ShaderBindingSetAllocator(CVulkanApplication& owner);
		void Create(ShaderBindingBuilder const& builder);
		std::shared_ptr<ShaderBindingSet> AllocateSet();
		virtual void Release() override;
	private:
		ShaderBindingSetMetadata m_Metadata;
		vk::DescriptorSetLayout m_DescriptorSetLayout;
		TVulkanApplicationPool<ShaderBindingSet_Impl> m_ShaderBindingSetPool;
	};
}