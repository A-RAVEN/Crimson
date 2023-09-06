#pragma once
#include <RenderInterface/header/ShaderBindingSet.h>
#include <RenderInterface/header/ShaderBindingBuilder.h>
#include "Containers.h"
#include "VulkanApplicationSubobjectBase.h"
#include "CVulkanBufferObject.h"

namespace graphics_backend
{
	class ShaderConstantSetMetadata;

	class ShaderConstantSet_Impl : public BaseUploadingResource, public ShaderConstantSet
	{
	public:
		ShaderConstantSet_Impl(CVulkanApplication& owner);
		// 通过 ShaderBindingSet 继承
		virtual void SetValue(std::string const& name, void* pValue) override;
		void Initialize(ShaderConstantSetMetadata const* inMetaData);
		virtual void UploadAsync() override;
		virtual bool UploadingDone() const override;
	protected:
		virtual void DoUpload() override;
	private:
		ShaderConstantSetMetadata const* p_Metadata;
		std::vector<uint8_t> m_UploadData;
		std::shared_ptr<CVulkanBufferObject> m_BufferObject;
	};

	class ShaderConstantSetMetadata
	{
	public:
		void Initialize(ShaderConstantsBuilder const& builder);
		std::unordered_map<std::string, std::pair<size_t, size_t>> const& GetArithmeticValuePositions() const;
		size_t GetTotalSize() const;
	private:
		ShaderConstantsBuilder const* p_Builder;
		size_t m_TotalSize = 0;
		std::unordered_map<std::string, std::pair<size_t, size_t>> m_ArithmeticValuePositions;
	};

	class ShaderConstantSetAllocator : public BaseApplicationSubobject
	{
	public:
		ShaderConstantSetAllocator(CVulkanApplication& owner);
		void Create(ShaderConstantsBuilder const& builder);
		std::shared_ptr<ShaderConstantSet> AllocateSet();
		virtual void Release() override;
	private:
		ShaderConstantSetMetadata m_Metadata;
		TVulkanApplicationPool<ShaderConstantSet_Impl> m_ShaderConstantSetPool;
	};
}