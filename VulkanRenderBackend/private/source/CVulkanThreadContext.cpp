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
		m_CommandPool == m_OwningApplication->GetDevice().createCommandPool(commandPoolCreateInfo);
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

	CVulkanThreadContext::CVulkanThreadContext(CVulkanApplication const* owningApplication)
	{
		Initialize(owningApplication);
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

	void CVulkanThreadContext::Initialize_Internal(CVulkanApplication const* owningApplication)
	{
		m_FrameBoundCommandBufferPools.resize(FRAMEBOUND_COMMANDPOOL_SWAP_COUNT_PER_CONTEXT);
		assert(m_FrameBoundCommandBufferPools.size() == 0);
		std::for_each(m_FrameBoundCommandBufferPools.begin(), m_FrameBoundCommandBufferPools.end()
			, [owningApplication](CVulkanFrameBoundCommandBufferPool& itrPool)
			{
				itrPool.Initialize(owningApplication);
			});
	}
	void CVulkanThreadContext::Release_Internal()
	{
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
