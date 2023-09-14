#pragma once
#include <SharedTools/header/uhash.h>
#include "VulkanIncludes.h"
#include "VulkanApplicationSubobjectBase.h"
#include <list>
#include <SharedTools/header/RAII.h>

namespace graphics_backend
{
	class DescriptorSetPool;
	class ShaderDescriptorSetAllocator;
	class ChunkedDescriptorPoolWrapper;

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
			, DescriptorSetPool& pool);
		ShaderDescriptorSetObject(ShaderDescriptorSetObject const& other) = delete;
		ShaderDescriptorSetObject& operator=(ShaderDescriptorSetObject const& other) = delete;
		ShaderDescriptorSetObject(ShaderDescriptorSetObject && other) = default;
		ShaderDescriptorSetObject& operator=(ShaderDescriptorSetObject && other) = default;
	private:

		friend class DescriptorSetPool;
		vk::DescriptorSet m_DescriptorSet;
		DescriptorSetPool& m_OwningPool;
	};

	using ShaderDescriptorSetHandle = raii_utils::TRAIIContainer<ShaderDescriptorSetObject>;

	class DescriptorSetPool : public BaseApplicationSubobject
	{
	public:
		DescriptorSetPool(CVulkanApplication& application
			, ChunkedDescriptorPoolWrapper const& owningWrapper
			, uint32_t maxSize);
		void Initialize();
		ShaderDescriptorSetHandle AllocateSet();
		bool Available() const { return m_UsingSize < m_MaxSize; }
		bool operator<(DescriptorSetPool const& other) const
		{
			return m_Pool < other.m_Pool;
		}
		bool operator==(DescriptorSetPool const& other) const
		{
			return m_Pool == other.m_Pool;
		}
	private:
		void ReleaseSet(ShaderDescriptorSetObject& releasedSet);
		vk::DescriptorPool m_Pool = nullptr;
		uint32_t m_UsingSize = 0;
		uint32_t m_MaxSize = 0;
		std::deque<vk::DescriptorSet> m_AvailableSets;
		ChunkedDescriptorPoolWrapper const& m_OwningWrapper;
	};

	class ChunkedDescriptorPoolWrapper : public BaseApplicationSubobject
	{
	public:
		ChunkedDescriptorPoolWrapper(ShaderDescriptorSetAllocator& owningAllocator
			, CVulkanApplication& application);
		void Initialize(ShaderDescriptorSetLayoutInfo, uint32_t chunkSize = 4);
		ShaderDescriptorSetHandle AllocateSet();
		uint32_t GetChunkSize() const { return m_ChunkSize; }
		ShaderDescriptorSetAllocator const& GetAllocator() const { return m_OwningAllocator; }
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

		ShaderDescriptorSetObject AllocateSet();

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