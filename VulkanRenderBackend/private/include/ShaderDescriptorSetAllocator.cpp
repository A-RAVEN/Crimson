#include "pch.h"
#include "ShaderDescriptorSetAllocator.h"

namespace graphics_backend
{
	ShaderDescriptorSetAllocator::ShaderDescriptorSetAllocator(CVulkanApplication& owner) : BaseApplicationSubobject(owner)
		, m_DescriptorPool(*this, owner)
	{
	}

	void ShaderDescriptorSetAllocator::Create(ShaderDescriptorSetLayoutInfo const& layoutInfo)
	{
		m_LayoutInfo = layoutInfo;
		uint32_t bindingCount = m_LayoutInfo.m_ConstantBufferCount;
		std::vector<vk::DescriptorSetLayoutBinding> bindings;
		bindings.resize(bindingCount);
		uint32_t bindingIndex = 0;
		for (uint32_t itrConstant = 0; itrConstant < m_LayoutInfo.m_ConstantBufferCount; ++itrConstant)
		{
			auto& binding = bindings[bindingIndex];
			binding.binding = bindingIndex;
			binding.descriptorCount = 1;
			binding.descriptorType = vk::DescriptorType::eUniformBuffer;
			binding.stageFlags = vk::ShaderStageFlagBits::eAll;
		}

		vk::DescriptorSetLayoutCreateInfo layoutCreateInfo{{}, bindings};

		m_DescriptorSetLayout = GetDevice().createDescriptorSetLayout(layoutCreateInfo);
	}
	void ShaderDescriptorSetAllocator::Release()
	{
		GetDevice().destroyDescriptorSetLayout(m_DescriptorSetLayout);
	}
	ChunkedDescriptorPoolWrapper::ChunkedDescriptorPoolWrapper(ShaderDescriptorSetAllocator& owningAllocator
		, CVulkanApplication& application) :
		BaseApplicationSubobject(application)
		, m_OwningAllocator(owningAllocator)
	{
	}

	ShaderDescriptorSetHandle ChunkedDescriptorPoolWrapper::AllocateSet()
	{
		return GetAvailablePool().AllocateSet();
	}


	DescriptorSetPool& ChunkedDescriptorPoolWrapper::GetAvailablePool()
	{
;
	}

	DescriptorSetPool::DescriptorSetPool(CVulkanApplication& application
		, ChunkedDescriptorPoolWrapper const& owningWrapper
		, uint32_t maxSize) : BaseApplicationSubobject(application)
		, m_OwningWrapper(owningWrapper)
	{
	}
	ShaderDescriptorSetHandle DescriptorSetPool::AllocateSet()
	{
		vk::DescriptorSet newSet;
		if (m_AvailableSets.empty())
		{
			std::array<vk::DescriptorSetLayout, 1> layouts = { m_OwningWrapper.GetAllocator().GetDescriptorSetLayout() };
			vk::DescriptorSetAllocateInfo allocateInfo{m_Pool, layouts};
			newSet = GetDevice().allocateDescriptorSets(allocateInfo).front();
		}
		else
		{
			newSet = m_AvailableSets.front();
			m_AvailableSets.pop_front();
		}
		++m_UsingSize;
		return ShaderDescriptorSetHandle{ ShaderDescriptorSetObject{ newSet , *this }, [this](ShaderDescriptorSetObject& releaseObj)
			{
				ReleaseSet(releaseObj);
			} };
	}

	void DescriptorSetPool::ReleaseSet(ShaderDescriptorSetObject& releasedSet)
	{
		m_AvailableSets.push_back(releasedSet.m_DescriptorSet);
		--m_UsingSize;
	}


	void DescriptorSetPool::Initialize()
	{
		auto& layoutInfo = m_OwningWrapper.GetAllocator().GetLayoutInfo();
		std::vector<vk::DescriptorPoolSize> poolSizes = {
			vk::DescriptorPoolSize{vk::DescriptorType::eUniformBuffer, layoutInfo.m_ConstantBufferCount}
		};
		vk::DescriptorPoolCreateInfo poolInfo{{}, m_OwningWrapper.GetChunkSize(), poolSizes};
		m_Pool = GetDevice().createDescriptorPool(poolInfo);
	}
	ShaderDescriptorSetObject::ShaderDescriptorSetObject(vk::DescriptorSet descriptorSet, DescriptorSetPool& pool) : 
		m_DescriptorSet(descriptorSet)
		, m_OwningPool(pool)
	{
	}
}
