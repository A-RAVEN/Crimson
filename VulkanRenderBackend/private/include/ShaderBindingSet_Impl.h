#pragma once
#include <RenderInterface/header/ShaderBindingSet.h>
#include <RenderInterface/header/ShaderBindingBuilder.h>
#include "Containers.h"
#include "VulkanApplicationSubobjectBase.h"
#include "CVulkanBufferObject.h"
#include "ShaderDescriptorSetAllocator.h"
#include "TickUploadingResource.h"
#include "GPUTexture_Impl.h"
#include "TextureSampler_Impl.h"

namespace graphics_backend
{
	class ShaderConstantSetMetadata;
	class ShaderDescriptorSetAllocator;

	class ShaderConstantSet_Impl : public BaseUploadingResource, public ShaderConstantSet
	{
	public:
		ShaderConstantSet_Impl(CVulkanApplication& owner);
		// 通过 ShaderBindingSet 继承
		virtual void SetValue(std::string const& name, void* pValue) override;
		void Initialize(ShaderConstantSetMetadata const* inMetaData);
		virtual void UploadAsync() override;
		virtual bool UploadingDone() const override;
		virtual std::string const& GetName() const override;
		std::shared_ptr<CVulkanBufferObject> GetBufferObject() const { return m_BufferObject; }
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
		ShaderConstantsBuilder const* GetBuilder() const { return p_Builder; }
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

	class ShaderBindingSetMetadata
	{
	public:
		void Initialize(ShaderBindingBuilder const& builder);
		ShaderDescriptorSetLayoutInfo const& GetLayoutInfo() const { return m_LayoutInfo; }
		std::unordered_map<std::string, uint32_t> const& GetCBufferNameToBindingIndex() const { return m_CBufferNameToBindingIndex; }
		std::unordered_map<std::string, uint32_t> const& GetTextureNameToBindingIndex() const { return m_TextureNameToBindingIndex; }
		
		uint32_t CBufferNameToBindingIndex(std::string const& cbufferName) const
		{
			auto it = m_CBufferNameToBindingIndex.find(cbufferName);
			if (it == m_CBufferNameToBindingIndex.end())
			{
				return std::numeric_limits<uint32_t>::max();
			}
			return it->second;
		}
		uint32_t TextureNameToBindingIndex(std::string const& textureName) const
		{
			auto it = m_TextureNameToBindingIndex.find(textureName);
			if (it == m_TextureNameToBindingIndex.end())
			{
				return std::numeric_limits<uint32_t>::max();
			}
			return it->second;
		}
		uint32_t SamplerNameToBindingIndex(std::string const& samplerName) const
		{
			auto it = m_SamplerNameToBindingIndex.find(samplerName);
			if (it == m_SamplerNameToBindingIndex.end())
			{
				return std::numeric_limits<uint32_t>::max();
			}
			return it->second;
		}
	private:
		ShaderDescriptorSetLayoutInfo m_LayoutInfo;
		std::unordered_map<std::string, uint32_t> m_CBufferNameToBindingIndex;
		std::unordered_map<std::string, uint32_t> m_TextureNameToBindingIndex;
		std::unordered_map<std::string, uint32_t> m_SamplerNameToBindingIndex;
	};



	class ShaderBindingSet_Impl : public BaseTickingUpdateResource, public ShaderBindingSet
	{
	public:
		ShaderBindingSet_Impl(CVulkanApplication& owner);
		void Initialize(ShaderBindingSetMetadata const* inMetaData);
		virtual void SetConstantSet(std::string const& name, std::shared_ptr<ShaderConstantSet> const& pConstantSet) override;
		virtual void SetTexture(std::string const& name
			, std::shared_ptr<GPUTexture> const& pTexture) override;
		virtual void SetSampler(std::string const& name
			, std::shared_ptr<TextureSampler> const& pSampler) override;
		virtual bool UploadingDone() const override;
		vk::DescriptorSet GetDescriptorSet() const {
			return m_DescriptorSetHandle->GetDescriptorSet();
		}
		virtual void TickUpload() override;
		ShaderBindingSetMetadata const* GetMetadata() const { return p_Metadata; }
	private:
		ShaderBindingSetMetadata const* p_Metadata;
		ShaderDescriptorSetHandle m_DescriptorSetHandle;
		std::unordered_map<std::string, std::shared_ptr<ShaderConstantSet>> m_ConstantSets;
		std::unordered_map<std::string, std::shared_ptr<GPUTexture_Impl>> m_Textures;
		std::unordered_map<std::string, std::shared_ptr<TextureSampler_Impl>> m_Samplers;
	};

	class ShaderBindingSetAllocator : public BaseApplicationSubobject
	{
	public:
		ShaderBindingSetAllocator(CVulkanApplication& owner);
		void Create(ShaderBindingBuilder const& builder);
		std::shared_ptr<ShaderBindingSet> AllocateSet();
		virtual void Release() override;
		virtual void TickUploadResources(CTaskGraph* pTaskGraph);
	private:
		ShaderBindingSetMetadata m_Metadata;
		TTickingUpdateResourcePool<ShaderBindingSet_Impl> m_ShaderBindingSetPool;
	};
}