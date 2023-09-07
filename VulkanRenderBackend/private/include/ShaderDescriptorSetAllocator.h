#pragma once
#include <SharedTools/header/uhash.h>
#include "VulkanIncludes.h"
#include "VulkanApplicationSubobjectBase.h"

namespace graphics_backend
{
	class ShaderDescriptorSetLayoutInfo
	{
	public:
		uint32_t m_ConstantBufferCount = 0;
		bool operator==(ShaderDescriptorSetLayoutInfo const& other) const
		{
			return hash_utils::memory_equal(*this, other);
		}
	};

	class ChunkedDescriptorPoolWrapper : public BaseApplicationSubobject
	{
	public:
		ChunkedDescriptorPoolWrapper(ShaderDescriptorSetAllocator& owningAllocator
			, CVulkanApplication& application);
		void Initialize(ShaderDescriptorSetLayoutInfo, uint32_t chunkSize = 4);
		vk::DescriptorSet AllocateSet();
		void ReleaseSet();
	private:
		vk::DescriptorPool GetAvailablePool();
		uint32_t m_ChunkSize = 1;
		ShaderDescriptorSetAllocator const& m_OwningAllocator;
		std::vector<vk::DescriptorPool> m_DescriptorSetPoolList;
	};

	class ShaderDescriptorSetAllocator : public BaseApplicationSubobject
	{
	public:
		ShaderDescriptorSetAllocator(CVulkanApplication& owner);
		void Create(ShaderDescriptorSetLayoutInfo const& layoutInfo);
		virtual void Release() override;

		vk::DescriptorSet AllocateSet();

		vk::DescriptorSetLayout GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }
		ShaderDescriptorSetLayoutInfo const& GetLayoutInfo() const { return m_LayoutInfo; }
	private:
		ShaderDescriptorSetLayoutInfo m_LayoutInfo;
		vk::DescriptorSetLayout m_DescriptorSetLayout;
		ChunkedDescriptorPoolWrapper m_DescriptorPool;
	};
}

template<>
struct hash_utils::is_contiguously_hashable<graphics_backend::ShaderDescriptorSetLayoutInfo> : public std::true_type {};