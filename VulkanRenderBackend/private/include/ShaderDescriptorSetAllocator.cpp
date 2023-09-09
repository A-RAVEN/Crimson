#include "pch.h"
#include "ShaderDescriptorSetAllocator.h"

namespace graphics_backend
{
	void ShaderDescriptorSetAllocator::Create(ShaderDescriptorSetLayoutInfo const& layoutInfo)
	{
		uint32_t bindingCount = layoutInfo.m_ConstantBufferCount;
		std::vector<vk::DescriptorSetLayoutBinding> bindings;
		bindings.resize(bindingCount);
		uint32_t bindingIndex = 0;
		for (uint32_t itrConstant = 0; itrConstant < layoutInfo.m_ConstantBufferCount; ++itrConstant)
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
	vk::DescriptorSet ChunkedDescriptorPoolWrapper::AllocateSet()
	{
		vk::DescriptorPool pool = m_DescriptorSetPoolList.back();

		vk::DescriptorSetAllocateInfo allocInfo{pool, 1, & m_OwningAllocator.GetDescriptorSetLayout()};
		vk::DescriptorSet result = GetDevice().allocateDescriptorSets(allocInfo).back();
		return result;
	}
	void ChunkedDescriptorPoolWrapper::ReleaseSet()
	{
		//GetDevice().releaseDes
	}
	DescriptorSetPool& ChunkedDescriptorPoolWrapper::GetAvailablePool()
	{
		auto& layoutInfo = m_OwningAllocator.GetLayoutInfo();
		std::vector<vk::DescriptorPoolSize> poolSizes = {
			vk::DescriptorPoolSize{vk::DescriptorType::eUniformBuffer, layoutInfo.m_ConstantBufferCount}
		};
		vk::DescriptorPoolCreateInfo poolInfo{{}, m_ChunkSize, poolSizes};
		GetDevice().createDescriptorPool(poolInfo);
		return vk::DescriptorPool();
	}

	DescriptorSetPool::DescriptorSetPool(CVulkanApplication& application, uint32_t maxSize) : BaseApplicationSubobject(application)
	{
	}
	ShaderDescriptorSetObject DescriptorSetPool::AllocateSet(ShaderDescriptorSetAllocator const& owningAllocator)
	{
	}
	void DescriptorSetPool::Initialize()
	{

	}
}
