#include <private/include/pch.h>
#include <private/include/RenderBackendSettings.h>
#include <private/include/CVulkanApplication.h>
#include <private/include/CVulkanThreadContext.h>

namespace graphics_backend
{
	vk::CommandBuffer CVulkanFrameBoundCommandBufferPool::AllocateCommandBuffer()
	{
		return m_CommandBufferList.AllocCommandBuffer(*this, false);
	}
	vk::CommandBuffer CVulkanFrameBoundCommandBufferPool::AllocateSecondaryCommandBuffer()
	{
		return m_SecondaryCommandBufferList.AllocCommandBuffer(*this, true);
	}
	void CVulkanFrameBoundCommandBufferPool::ResetCommandBufferPool()
	{
		GetDevice().resetCommandPool(m_CommandPool, vk::CommandPoolResetFlagBits::eReleaseResources);
		m_CommandBufferList.ResetBufferList();
		m_SecondaryCommandBufferList.ResetBufferList();
	}
	void CVulkanFrameBoundCommandBufferPool::CollectCommandBufferList(std::vector<vk::CommandBuffer>& inoutCommandBufferList)
	{
		size_t commandCount = m_CommandBufferList.m_AvailableCommandBufferIndex + m_SecondaryCommandBufferList.m_AvailableCommandBufferIndex;
		if (commandCount > 0)
		{
			inoutCommandBufferList.reserve(inoutCommandBufferList.size() + commandCount);
			m_CommandBufferList.CollectCommandBufferList(inoutCommandBufferList);
			m_SecondaryCommandBufferList.CollectCommandBufferList(inoutCommandBufferList);
		}
	}
	void CVulkanFrameBoundCommandBufferPool::Initialize_Internal(CVulkanApplication const* owningApplication)
	{
		vk::CommandPoolCreateInfo commandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eTransient, 0);
		m_CommandPool = m_OwningApplication->GetDevice().createCommandPool(commandPoolCreateInfo);
		m_CommandBufferList.ClearBufferList();
		m_SecondaryCommandBufferList.ClearBufferList();
	}

	void CVulkanFrameBoundCommandBufferPool::Release_Internal()
	{
		m_CommandBufferList.ClearBufferList();
		m_SecondaryCommandBufferList.ClearBufferList();
		m_OwningApplication->GetDevice().destroyCommandPool(m_CommandPool);
		m_CommandPool = nullptr;
	}

	CVulkanFrameBoundCommandBufferPool& CVulkanThreadContext::GetCurrentFramePool()
	{
		uint32_t currentFrameId =GetVulkanApplication()
			->GetSubmitCounterContext().GetCurrentFrameBufferIndex();
		return m_FrameBoundCommandBufferPools[currentFrameId];
	}

	void CVulkanThreadContext::CollectSubmittingCommandBuffers(std::vector<vk::CommandBuffer>& inoutCommandBufferList)
	{
		GetCurrentFramePool().CollectCommandBufferList(inoutCommandBufferList);
	}

	CVulkanBufferObject const& CVulkanThreadContext::AllocBufferObject(bool gpuBuffer, uint32_t bufferSize, vk::BufferUsageFlags bufferUsage)
	{
		CVulkanBufferObject result = GetVulkanApplication()->SubObject<CVulkanBufferObject>();

		VkBufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo(
			{}, bufferSize, bufferUsage, vk::SharingMode::eExclusive
		);
		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = gpuBuffer ? VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE : VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
		VkBuffer vkbuffer_c;
		vmaCreateBuffer(m_ThreadGPUAllocator, &bufferCreateInfo, &allocationCreateInfo, &vkbuffer_c, &result.m_BufferAllocation, &result.m_BufferAllocationInfo);
		result.m_Buffer = vkbuffer_c;
		return result;
	}

	void CVulkanThreadContext::Initialize_Internal(CVulkanApplication const* owningApplication)
	{
		assert(m_FrameBoundCommandBufferPools.size() == 0);
		m_FrameBoundCommandBufferPools.resize(FRAMEBOUND_COMMANDPOOL_SWAP_COUNT_PER_CONTEXT);
		std::for_each(m_FrameBoundCommandBufferPools.begin(), m_FrameBoundCommandBufferPools.end()
			, [owningApplication](CVulkanFrameBoundCommandBufferPool& itrPool)
			{
				itrPool.Initialize(owningApplication);
			});

		VmaAllocatorCreateInfo vmaCreateInfo{};
		vmaCreateInfo.vulkanApiVersion = VULKAN_API_VERSION_IN_USE;
		vmaCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;
		vmaCreateInfo.physicalDevice = static_cast<VkPhysicalDevice>(GetPhysicalDevice());
		vmaCreateInfo.device = GetDevice();
		vmaCreateInfo.instance = GetInstance();
		vmaCreateAllocator(&vmaCreateInfo, &m_ThreadGPUAllocator);
	}
	void CVulkanThreadContext::Release_Internal()
	{
		vmaDestroyAllocator(m_ThreadGPUAllocator);
		m_ThreadGPUAllocator = nullptr;

		if (!m_FrameBoundCommandBufferPools.empty())
		{
			std::for_each(m_FrameBoundCommandBufferPools.begin(), m_FrameBoundCommandBufferPools.end()
				, [](CVulkanFrameBoundCommandBufferPool& itrPool)
				{
					itrPool.Release();
				});
		}
	}
	vk::CommandBuffer CVulkanFrameBoundCommandBufferPool::CommandBufferList::AllocCommandBuffer(CVulkanFrameBoundCommandBufferPool& owner, bool secondary)
	{
		if (m_AvailableCommandBufferIndex < m_CommandBufferList.size())
		{
			size_t newCmdId = m_AvailableCommandBufferIndex;
			++m_AvailableCommandBufferIndex;
			return m_CommandBufferList[newCmdId];
		}
		vk::CommandBuffer newCmd = owner.GetDevice()
			.allocateCommandBuffers(
				vk::CommandBufferAllocateInfo(owner.m_CommandPool
					, secondary ? vk::CommandBufferLevel::eSecondary : vk::CommandBufferLevel::ePrimary
					, 1u)).front();
		m_CommandBufferList.push_back(newCmd);
		++m_AvailableCommandBufferIndex;
		return newCmd;
	}
	void CVulkanFrameBoundCommandBufferPool::CommandBufferList::CollectCommandBufferList(std::vector<vk::CommandBuffer>& inoutCommandBufferList)
	{
		for (int i = 0; i < m_AvailableCommandBufferIndex; ++i)
		{
			inoutCommandBufferList.push_back(m_CommandBufferList[i]);
		}
	}
	void CVulkanFrameBoundCommandBufferPool::CommandBufferList::ResetBufferList()
	{
		m_AvailableCommandBufferIndex = 0;
	}
	void CVulkanFrameBoundCommandBufferPool::CommandBufferList::ClearBufferList()
	{
		m_AvailableCommandBufferIndex = 0;
		m_CommandBufferList.clear();
	}
}
