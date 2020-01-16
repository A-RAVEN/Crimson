#include <headers/VulkanGPUDeviceThread.h>
#include <headers/VulkanGPUDevice.h>
#include <headers/VulkanDebugLog.h>
#include <headers/VulkanGraphicsCommandBuffer.h>
#include <headers/VulkanRenderPassInstance.h>
#include <headers/GeneralDebug.h>

namespace Crimson
{
	VulkanGPUDeviceThread::VulkanGPUDeviceThread() : 
		p_OwningDevice(nullptr),
		m_GraphicsCommandPool(VK_NULL_HANDLE),
		m_ComputeCommandPool(VK_NULL_HANDLE)
	{}
	VulkanGPUDeviceThread::~VulkanGPUDeviceThread()
	{
	}
	void VulkanGPUDeviceThread::Dispose()
	{
		if (m_GraphicsCommandPool)
		{
			vkDestroyCommandPool(p_OwningDevice->m_LogicalDevice, m_GraphicsCommandPool, VULKAN_ALLOCATOR_POINTER);
		}
		if (m_ComputeCommandPool)
		{
			vkDestroyCommandPool(p_OwningDevice->m_LogicalDevice, m_ComputeCommandPool, VULKAN_ALLOCATOR_POINTER);
		}
		p_OwningDevice->HandleDisposedThread(this);
	}
	PGraphicsCommandBuffer VulkanGPUDeviceThread::StartSubpassCommand(PRenderPassInstance renderpass_instance, uint32_t subpass_id)
	{
		InitGraphicsCommandPool();
		VulkanRenderPassInstance* vulkan_renderpass_instance = static_cast<VulkanRenderPassInstance*>(renderpass_instance);
		CRIM_ASSERT(vulkan_renderpass_instance->m_InstanceUniqueId != NUMMAX_UINT64, "Vulkan Start Subpass Command Encounter Invalid RenderPassInstance");
		if (m_RenderPassInstanceGraphicsCommandBufferInfoReferences.size() <= vulkan_renderpass_instance->m_InstanceUniqueId)
		{
			size_t original_size = m_RenderPassInstanceGraphicsCommandBufferInfoReferences.size();
			m_RenderPassInstanceGraphicsCommandBufferInfoReferences.resize(vulkan_renderpass_instance->m_InstanceUniqueId + 1);
			for (size_t start_id = original_size; start_id < m_RenderPassInstanceGraphicsCommandBufferInfoReferences.size(); ++start_id)
			{
				m_RenderPassInstanceGraphicsCommandBufferInfoReferences[start_id] = NUMMAX_UINT32;
			}
		}
		if (m_RenderPassInstanceGraphicsCommandBufferInfoReferences[vulkan_renderpass_instance->m_InstanceUniqueId] == NUMMAX_UINT32)
		{
			m_RenderPassInstanceGraphicsCommandBufferInfos.push_back(RenderPassInstanceGraphicsCommandBufferInfo{});
			m_RenderPassInstanceGraphicsCommandBufferInfos.rbegin()->InitBufferInfo(vulkan_renderpass_instance);
			m_RenderPassInstanceGraphicsCommandBufferInfoReferences[vulkan_renderpass_instance->m_InstanceUniqueId] = m_RenderPassInstanceGraphicsCommandBufferInfos.size() - 1;
		}
		auto& cmd_buffer_info = m_RenderPassInstanceGraphicsCommandBufferInfos[m_RenderPassInstanceGraphicsCommandBufferInfoReferences[vulkan_renderpass_instance->m_InstanceUniqueId]];
		if (cmd_buffer_info.m_SubpassCommands[subpass_id] != VK_NULL_HANDLE)
		{
			m_RecycledGraphicsCommandBuffer.push_back(cmd_buffer_info.m_SubpassCommands[subpass_id]);
		}
		//TODO add allocation
		VkCommandBufferAllocateInfo allocate_info{};
		allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocate_info.commandPool = m_GraphicsCommandPool;
		allocate_info.commandBufferCount = 1;
		allocate_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		allocate_info.pNext = nullptr;
		VkCommandBuffer new_cmd_buffer = VK_NULL_HANDLE;
		vkAllocateCommandBuffers(p_OwningDevice->m_LogicalDevice, &allocate_info, &new_cmd_buffer);

		cmd_buffer_info.m_SubpassCommands[subpass_id] = new_cmd_buffer;
		VulkanGraphicsCommandBuffer* return_val = new VulkanGraphicsCommandBuffer();
		return_val->SetGraphicsCommandBuffer(this, 
			static_cast<VulkanRenderPass*>(vulkan_renderpass_instance->p_RenderPass), vulkan_renderpass_instance, subpass_id, new_cmd_buffer);
		return_val->StartCommandBuffer();
		return return_val;
	}
	PExecutionCommandBuffer VulkanGPUDeviceThread::CreateExecutionCommandBuffer(EExecutionCommandType cmd_type)
	{
		switch (cmd_type)
		{
		case EExecutionCommandType::E_COMMAND_TYPE_GRAPHICS:
			InitGraphicsCommandPool();
			break;
		case EExecutionCommandType::E_COMMAND_TYPE_COMPUTE:
			InitComputeCommandPool();
			break;
		default:
			break;
		}

		return PExecutionCommandBuffer();
	}
	void VulkanGPUDeviceThread::OnGraphicsCommandBufferFinished(VulkanGraphicsCommandBuffer* cmd_buffer)
	{
		delete cmd_buffer;
	}
	void VulkanGPUDeviceThread::InitGPUDeviceThread(VulkanGPUDevice* device)
	{
		p_OwningDevice = device;
	}
	void VulkanGPUDeviceThread::InitGraphicsCommandPool()
	{
		if (m_GraphicsCommandPool == VK_NULL_HANDLE)
		{
			VkCommandPoolCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			create_info.queueFamilyIndex = p_OwningDevice->m_GraphicsDedicateFamily;
			create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			create_info.pNext = nullptr;
			VulkanDebug::CheckVKResult(vkCreateCommandPool(p_OwningDevice->m_LogicalDevice, &create_info,
				VULKAN_ALLOCATOR_POINTER, &m_GraphicsCommandPool), "Vulkan Create Graphics Command Pool Issue!");
		}
	}
	void VulkanGPUDeviceThread::InitComputeCommandPool()
	{
		if (m_ComputeCommandPool == VK_NULL_HANDLE) {
			VkCommandPoolCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			create_info.queueFamilyIndex = p_OwningDevice->m_ComputeDedicateFamily;
			create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			create_info.pNext = nullptr;
			VulkanDebug::CheckVKResult(vkCreateCommandPool(p_OwningDevice->m_LogicalDevice, &create_info,
				VULKAN_ALLOCATOR_POINTER, &m_ComputeCommandPool), "Vulkan Create Compute Command Pool Issue!");
		}
	}
	VkCommandBuffer VulkanGPUDeviceThread::AllocExecutionVkCommandBuffer(EExecutionCommandType cmd_type)
	{
		switch (cmd_type)
		{
		case EExecutionCommandType::E_COMMAND_TYPE_GRAPHICS:
			if (!m_RecycledGraphicsExecutionCommandBuffer.empty())
			{
				auto return_val = m_RecycledGraphicsExecutionCommandBuffer.front();
				CHECK_VKRESULT(vkResetCommandBuffer(return_val, VkCommandBufferResetFlagBits::VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT), "Vulkan Reset Recycled Command Buffer Issue!");
				m_RecycledGraphicsExecutionCommandBuffer.pop_front();
				return return_val;
			}
		case EExecutionCommandType::E_COMMAND_TYPE_COMPUTE:
			if (!m_RecycledComputeExecutionCommandBuffer.empty())
			{
				auto return_val = m_RecycledComputeExecutionCommandBuffer.front();
				CHECK_VKRESULT(vkResetCommandBuffer(return_val, VkCommandBufferResetFlagBits::VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT), "Vulkan Reset Recycled Command Buffer Issue!");
				m_RecycledComputeExecutionCommandBuffer.pop_front();
				return return_val;
			}
		}

		VkCommandBufferAllocateInfo  new_alloc_info{};
		new_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		new_alloc_info.commandPool = cmd_type == EExecutionCommandType::E_COMMAND_TYPE_GRAPHICS ? m_GraphicsCommandPool : m_ComputeCommandPool;
		new_alloc_info.commandBufferCount = 1;
		new_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		new_alloc_info.pNext = nullptr;
		VkCommandBuffer new_cmd_buffer = VK_NULL_HANDLE;
		CHECK_VKRESULT(vkAllocateCommandBuffers(p_OwningDevice->m_LogicalDevice, &new_alloc_info, &new_cmd_buffer), "Vulkan Allocate Primary Command Buffer Issue!");

		return new_cmd_buffer;
	}
	void VulkanGPUDeviceThread::RecycleExecutionVkCommandBuffer(VkCommandBuffer cmd_buffer, EExecutionCommandType cmd_type)
	{
		switch (cmd_type)
		{
		case EExecutionCommandType::E_COMMAND_TYPE_GRAPHICS:
			m_RecycledGraphicsExecutionCommandBuffer.push_back(cmd_buffer);
			break;
		case EExecutionCommandType::E_COMMAND_TYPE_COMPUTE:
			m_RecycledComputeExecutionCommandBuffer.push_back(cmd_buffer);
			break;
		}
	}
	RenderPassInstanceGraphicsCommandBufferInfo::RenderPassInstanceGraphicsCommandBufferInfo():
		m_SubpassCommands(0)
	{}
	void RenderPassInstanceGraphicsCommandBufferInfo::InitBufferInfo(VulkanRenderPassInstance* p_instance)
	{
		m_SubpassCommands = std::vector<VkCommandBuffer>(p_instance->p_RenderPass->m_Subpasses.size(), VK_NULL_HANDLE);
	}
}