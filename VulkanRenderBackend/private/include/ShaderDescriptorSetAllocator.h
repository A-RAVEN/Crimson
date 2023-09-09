#pragma once
#include <SharedTools/header/uhash.h>
#include "VulkanIncludes.h"
#include "VulkanApplicationSubobjectBase.h"
#include <list>

namespace graphics_backend
{
	class DescriptorSetPool;
	class ShaderDescriptorSetAllocator;

	class ShaderDescriptorSetLayoutInfo
	{
	public:
		uint32_t m_ConstantBufferCount = 0;
		bool operator==(ShaderDescriptorSetLayoutInfo const& other) const
		{
			return hash_utils::memory_equal(*this, other);
		}
	};

	class ShaderDescriptorSetObject
	{
	public:
		ShaderDescriptorSetObject(vk::DescriptorSet descriptorSet
			, ChunkedDescriptorPoolWrapper* pPoolWrapper
			, vk::DescriptorPool pool);
	private:
		friend class ChunkedDescriptorPoolWrapper;

		vk::DescriptorSet m_DescriptorSet = nullptr;
		ChunkedDescriptorPoolWrapper& const m_OwningPoolWrapper;
		DescriptorSetPool& m_OwningPool;
	};

	class DescriptorSetPool : public BaseApplicationSubobject
	{
	public:
		DescriptorSetPool(CVulkanApplication& application, uint32_t maxSize);
		void Initialize();
		ShaderDescriptorSetObject AllocateSet(ShaderDescriptorSetAllocator const& owningAllocator);
		void ReleaseSet(ShaderDescriptorSetObject& releasedSet);
		bool operator<(DescriptorSetPool const& other) const
		{
			return m_Pool < other.m_Pool;
		}
		bool operator==(DescriptorSetPool const& other) const
		{
			return m_Pool == other.m_Pool;
		}
	private:
		vk::DescriptorPool m_Pool = nullptr;
		uint32_t m_UsingSize = 0;
		uint32_t m_MaxSize = 0;
		std::deque<vk::DescriptorSet> m_AvailableSets;
	};

	class ChunkedDescriptorPoolWrapper : public BaseApplicationSubobject
	{
	public:
		ChunkedDescriptorPoolWrapper(ShaderDescriptorSetAllocator& owningAllocator
			, CVulkanApplication& application);
		void Initialize(ShaderDescriptorSetLayoutInfo, uint32_t chunkSize = 4);
		ShaderDescriptorSetObject AllocateSet();
		void ReleaseSet();
	private:
		DescriptorSetPool& GetAvailablePool();
		uint32_t m_ChunkSize = 1;
		ShaderDescriptorSetAllocator const& m_OwningAllocator;
		std::set<DescriptorSetPool> m_DescriptorSetPoolList;
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