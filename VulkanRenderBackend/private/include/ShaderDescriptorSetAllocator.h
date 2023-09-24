#pragma once
#include <SharedTools/header/uhash.h>
#include "VulkanIncludes.h"
#include "VulkanApplicationSubobjectBase.h"
#include <list>
#include <SharedTools/header/RAII.h>
#include "HashPool.h"
#include <RenderInterface/header/ShaderBindingBuilder.h>
#include "Containers.h"

namespace graphics_backend
{
	class DescriptorSetPool;
	class ShaderDescriptorSetAllocator;

	class ShaderDescriptorSetLayoutInfo
	{
	public:
		ShaderDescriptorSetLayoutInfo() = default;
		ShaderDescriptorSetLayoutInfo(ShaderBindingBuilder const& bindingBuilder);
		uint32_t m_ConstantBufferCount = 0;
		bool operator==(ShaderDescriptorSetLayoutInfo const& other) const
		{
			return hash_utils::memory_equal(*this, other);
		}
	};

	class ShaderDescriptorSetObject
	{
	public:
		ShaderDescriptorSetObject() = default;
		ShaderDescriptorSetObject(vk::DescriptorSet descriptorSet);
		vk::DescriptorSet GetDescriptorSet() const { return m_DescriptorSet; }
		ShaderDescriptorSetObject(ShaderDescriptorSetObject&& other) = default;
		ShaderDescriptorSetObject& operator=(ShaderDescriptorSetObject&& other) = default;
		ShaderDescriptorSetObject(ShaderDescriptorSetObject const& other) = default;
		ShaderDescriptorSetObject& operator=(ShaderDescriptorSetObject const& other) = default;
	private:
		friend class ChunkedDescriptorPoolWrapper;
		friend class DescriptorSetPool;
		vk::DescriptorSet m_DescriptorSet = nullptr;
	};

	using ShaderDescriptorSetHandle = raii_utils::TRAIIContainer<ShaderDescriptorSetObject>;

	class DescriptorSetPool : public BaseApplicationSubobject
	{
	public:
		DescriptorSetPool(CVulkanApplication& application
			, ShaderDescriptorSetAllocator const& owningAllocator
			, uint32_t maxSize);
		void Initialize();
		ShaderDescriptorSetHandle AllocateSet();
		void ClientReleaseSet(ShaderDescriptorSetObject&& releasedSet);
		void ReleaseFrameboundResources();
		bool IsFull() const { return m_AvailableSets.empty() && (m_UsingSize >= m_MaxSize); }
		bool operator<(DescriptorSetPool const& other) const
		{
			return m_Pool < other.m_Pool;
		}
		bool operator==(DescriptorSetPool const& other) const
		{
			return m_Pool == other.m_Pool;
		}
	private:
		ShaderDescriptorSetAllocator const& m_OwningAllocator;
		vk::DescriptorPool m_Pool = nullptr;
		uint32_t m_UsingSize = 0;
		uint32_t m_MaxSize = 0;
		std::deque<vk::DescriptorSet> m_AvailableSets;
		TFrameboundReleaser<ShaderDescriptorSetObject> m_FrameboundReleaser;
	};

	class ChunkedDescriptorPoolWrapper : public BaseApplicationSubobject
	{
	public:
		ChunkedDescriptorPoolWrapper(ShaderDescriptorSetAllocator& owningAllocator
			, CVulkanApplication& application
			, uint32_t chunkSize);
		ShaderDescriptorSetHandle AllocateSet();
		void Release() override;

		void ReleaseFrameboundResources();
	private:
		DescriptorSetPool& GetAvailablePool();
		void MarkPoolAvailable(DescriptorSetPool& pool);
		uint32_t m_ChunkSize = 1;
		ShaderDescriptorSetAllocator const& m_OwningAllocator;
		std::deque<DescriptorSetPool> m_DescriptorSetPoolList;
		std::deque<DescriptorSetPool*> m_AvailablePools;
	};

	class ShaderDescriptorSetAllocator : public BaseApplicationSubobject
	{
	public:
		ShaderDescriptorSetAllocator(CVulkanApplication& owner);
		void Create(ShaderDescriptorSetLayoutInfo const& layoutInfo);
		virtual void Release() override;

		ShaderDescriptorSetHandle AllocateSet();

		vk::DescriptorSetLayout GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }
		ShaderDescriptorSetLayoutInfo const& GetLayoutInfo() const { return *m_LayoutInfo; }

		void ReleaseFrameboundResources() { m_DescriptorPool.ReleaseFrameboundResources(); }
	private:
		ShaderDescriptorSetLayoutInfo const* m_LayoutInfo;
		vk::DescriptorSetLayout m_DescriptorSetLayout;
		ChunkedDescriptorPoolWrapper m_DescriptorPool;
	};

	using ShaderDescriptorSetAllocatorPool = HashPool<ShaderDescriptorSetLayoutInfo, ShaderDescriptorSetAllocator>;
}

template<>
struct hash_utils::is_contiguously_hashable<graphics_backend::ShaderDescriptorSetLayoutInfo> : public std::true_type {};