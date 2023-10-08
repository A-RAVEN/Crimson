#include "private/include/pch.h"
#include "ShaderDescriptorSetAllocator.h"
#include "FrameCountContext.h"
#include <SharedTools/header/DebugUtils.h>

namespace graphics_backend
{
	ShaderDescriptorSetAllocator::ShaderDescriptorSetAllocator(CVulkanApplication& owner) : BaseApplicationSubobject(owner)
	, m_DescriptorPool(*this, owner, 4)
	{
	}
	void ShaderDescriptorSetAllocator::Create(ShaderDescriptorSetLayoutInfo const& layoutInfo)
	{
		m_LayoutInfo = &layoutInfo;
		uint32_t constantBufferCount = m_LayoutInfo->m_ConstantBufferCount;
		uint32_t textureCount = m_LayoutInfo->m_TextureCount;
		uint32_t samplerCount = m_LayoutInfo->m_SamplerCount;
		uint32_t bindingCount = constantBufferCount + textureCount + samplerCount;
		std::vector<vk::DescriptorSetLayoutBinding> bindings;
		bindings.resize(bindingCount);
		uint32_t bindingIndex = 0;
		for (uint32_t itrConstant = 0; itrConstant < m_LayoutInfo->m_ConstantBufferCount; ++itrConstant)
		{
			auto& binding = bindings[bindingIndex];
			binding.binding = bindingIndex;
			binding.descriptorCount = 1;
			binding.descriptorType = vk::DescriptorType::eUniformBuffer;
			binding.stageFlags = vk::ShaderStageFlagBits::eAll;
			++bindingIndex;
		}

		for (uint32_t itrTexture = 0; itrTexture < m_LayoutInfo->m_TextureCount; ++itrTexture)
		{
			auto& binding = bindings[bindingIndex];
			binding.binding = bindingIndex;
			binding.descriptorCount = 1;
			binding.descriptorType = vk::DescriptorType::eSampledImage;
			binding.stageFlags = vk::ShaderStageFlagBits::eAll;
			++bindingIndex;
		}

		for (uint32_t itrSampler = 0; itrSampler < m_LayoutInfo->m_SamplerCount; ++itrSampler)
		{
			auto& binding = bindings[bindingIndex];
			binding.binding = bindingIndex;
			binding.descriptorCount = 1;
			binding.descriptorType = vk::DescriptorType::eSampler;
			binding.stageFlags = vk::ShaderStageFlagBits::eAll;
			++bindingIndex;
		}

		vk::DescriptorSetLayoutCreateInfo layoutCreateInfo{{}, bindings};

		m_DescriptorSetLayout = GetDevice().createDescriptorSetLayout(layoutCreateInfo);
	}
	void ShaderDescriptorSetAllocator::Release()
	{
		m_DescriptorPool.Release();
		GetDevice().destroyDescriptorSetLayout(m_DescriptorSetLayout);
	}
	ShaderDescriptorSetHandle ShaderDescriptorSetAllocator::AllocateSet()
	{
		return m_DescriptorPool.AllocateSet();
	}
	ChunkedDescriptorPoolWrapper::ChunkedDescriptorPoolWrapper(ShaderDescriptorSetAllocator& owningAllocator
		, CVulkanApplication& application
		, uint32_t chunkSize) :
		BaseApplicationSubobject(application)
		, m_OwningAllocator(owningAllocator)
		, m_ChunkSize(chunkSize)
	{
	}
	ShaderDescriptorSetHandle ChunkedDescriptorPoolWrapper::AllocateSet()
	{
		auto& pool = GetAvailablePool();
		CA_ASSERT(!pool.IsFull(), "Descriptor Pool Is Full!");
		ShaderDescriptorSetHandle result = pool.AllocateSet();
		if (!pool.IsFull())
		{
			MarkPoolAvailable(pool);
		}
		return result;
	}

	void ChunkedDescriptorPoolWrapper::Release()
	{
		for(auto& pool : m_DescriptorSetPoolList)
		{
			pool.Release();
		}
		m_DescriptorSetPoolList.clear();
		m_AvailablePools.clear();
	}

	void ChunkedDescriptorPoolWrapper::ReleaseFrameboundResources()
	{
		for (auto& pool : m_DescriptorSetPoolList)
		{
			bool isfullPreRelease = pool.IsFull();
			pool.ReleaseFrameboundResources();
			if (isfullPreRelease && !pool.IsFull())
			{
				MarkPoolAvailable(pool);
			};
		}
	}

	DescriptorSetPool& ChunkedDescriptorPoolWrapper::GetAvailablePool()
	{
		DescriptorSetPool* resultPool = nullptr;
		if (m_AvailablePools.empty())
		{
			m_DescriptorSetPoolList.emplace_back(GetVulkanApplication(), m_OwningAllocator, m_ChunkSize);
			m_DescriptorSetPoolList.back().Initialize();
			resultPool = &m_DescriptorSetPoolList.back();
		}
		else
		{
			resultPool = m_AvailablePools.back();
			m_AvailablePools.pop_back();
		}
		return *resultPool;
	}

	void ChunkedDescriptorPoolWrapper::MarkPoolAvailable(DescriptorSetPool& pool)
	{
		m_AvailablePools.push_back(&pool);
	}

	DescriptorSetPool::DescriptorSetPool(CVulkanApplication& application
		, ShaderDescriptorSetAllocator const& owningAllocator
		, uint32_t maxSize) : BaseApplicationSubobject(application)
		, m_OwningAllocator(owningAllocator)
		, m_MaxSize(maxSize)
		, m_FrameboundReleaser([this](std::deque<ShaderDescriptorSetObject> const& released)
			{
				for (auto& releasedSet : released)
				{
					m_AvailableSets.push_back(releasedSet.m_DescriptorSet);
				}
			})
	{
	}

	void DescriptorSetPool::Initialize()
	{
		auto& layoutInfo = m_OwningAllocator.GetLayoutInfo();
		std::vector<vk::DescriptorPoolSize> poolSizes = {
			vk::DescriptorPoolSize{vk::DescriptorType::eUniformBuffer, layoutInfo.m_ConstantBufferCount * m_MaxSize}
		};
		if (layoutInfo.m_TextureCount > 0)
		{
			poolSizes.emplace_back(
				vk::DescriptorType::eSampledImage
				, layoutInfo.m_TextureCount * m_MaxSize
			);
		}
		if (layoutInfo.m_SamplerCount > 0)
		{
			poolSizes.emplace_back(
				vk::DescriptorType::eSampler
				, layoutInfo.m_SamplerCount * m_MaxSize
			);
		}
		vk::DescriptorPoolCreateInfo poolInfo{{}, m_MaxSize, poolSizes};
		m_Pool = GetDevice().createDescriptorPool(poolInfo);
	}
	ShaderDescriptorSetHandle DescriptorSetPool::AllocateSet()
	{
		CA_ASSERT(!IsFull(), "Descriptor Set Pool Is Full!");
		vk::DescriptorSet resultSet = nullptr;
		if (!m_AvailableSets.empty())
		{
			resultSet = m_AvailableSets.front();
			m_AvailableSets.pop_front();
		}
		else
		{
			vk::DescriptorSetLayout layout = m_OwningAllocator.GetDescriptorSetLayout();
			vk::DescriptorSetAllocateInfo allocInfo{
				m_Pool
				, 1
				, &layout};
			resultSet = GetDevice().allocateDescriptorSets(allocInfo).front();
			++m_UsingSize;
		}
		ShaderDescriptorSetHandle result{ ShaderDescriptorSetObject(resultSet) , [this](ShaderDescriptorSetObject& released) {
			ClientReleaseSet(std::move(released));
		} };
		return result;
	}
	void DescriptorSetPool::ClientReleaseSet(ShaderDescriptorSetObject&& releasedSet)
	{
		FrameType currentFrame = GetFrameCountContext().GetCurrentFrameID();
		m_FrameboundReleaser.ScheduleRelease(currentFrame, std::move(releasedSet));
	}
	void DescriptorSetPool::ReleaseFrameboundResources()
	{
		FrameType releasingFrame = GetFrameCountContext().GetReleasedFrameID();
		m_FrameboundReleaser.ReleaseFrame(releasingFrame);
	}
	ShaderDescriptorSetObject::ShaderDescriptorSetObject(vk::DescriptorSet descriptorSet) :
		m_DescriptorSet(descriptorSet)
	{
	}
	ShaderDescriptorSetLayoutInfo::ShaderDescriptorSetLayoutInfo(ShaderBindingBuilder const& bindingBuilder) :
		m_ConstantBufferCount(bindingBuilder.GetConstantBufferDescriptors().size())
		, m_TextureCount(bindingBuilder.GetTextureDescriptors().size())
		, m_SamplerCount(bindingBuilder.GetTextureSamplers().size())
	{
	}
}