#include <headers/VulkanDescriptors.h>
#include <headers/VulkanDebugLog.h>
#include <headers/VulkanTranslator.h>
#include <headers/VulkanBuffer.h>

namespace Crimson
{
	VulkanDescriptorSet::VulkanDescriptorSet() : 
		p_OwningDevice(nullptr),
		m_DescriptorSet(VK_NULL_HANDLE)
	{}
	void VulkanDescriptorSet::WriteDescriptorSetBuffer(EBufferUniformType uniform_type, PGPUBuffer buffer, uint64_t buffer_offset, uint64_t write_size)
	{
		VulkanBufferObject* p_vulkan_buffer = static_cast<VulkanBufferObject*>(buffer);
		VkDescriptorBufferInfo new_buffer_write_info{};
		new_buffer_write_info.buffer = p_vulkan_buffer->m_Buffer;
		new_buffer_write_info.offset = buffer_offset;
		new_buffer_write_info.range = write_size;
		m_BufferWriteCache[static_cast<size_t>(uniform_type)].push_back(new_buffer_write_info);
	}
	void VulkanDescriptorSet::EndWriteDescriptorSet()
	{
		for (uint32_t enum_id = 0; enum_id < static_cast<uint32_t>(EBufferUniformType::E_BUFFER_UNIFORM_TYPE_MAX); ++enum_id)
		{
			EBufferUniformType type = EBufferUniformType(enum_id);
			if (m_BufferWriteCache[enum_id].size() > 0)
			{
				VkWriteDescriptorSet write_info{};
				write_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write_info.descriptorCount = m_BufferWriteCache[enum_id].size();
				//TODO: Change To Translation Function
				write_info.descriptorType = type == EBufferUniformType::E_BUFFER_UNIFORM_TYPE_UNIFORM_BUFFER ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				write_info.dstArrayElement = 0;
				write_info.dstBinding = 0;
				write_info.dstSet = m_DescriptorSet;
			}
		}
	}
	void VulkanDescriptorSet::SetVulkanDescriptorSet(VulkanGPUDevice* device, VkDescriptorSet set)
	{
		p_OwningDevice = device;
		m_DescriptorSet = set;
	}

	VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanGPUDevice* device) :
		p_OwningDevice(device),
		m_DescriptorSetLayout(VK_NULL_HANDLE)
	{}

	PDescriptorSet VulkanDescriptorSetLayout::AllocDescriptorSet()
	{
		BuildLayout();
		VulkanDescriptorSet* return_val = new VulkanDescriptorSet();
		VkDescriptorSet new_set = VK_NULL_HANDLE;
		VkDescriptorSetAllocateInfo allocate_info{};
		allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocate_info.descriptorPool = p_OwningDevice->m_DescriptorPool;
		allocate_info.descriptorSetCount = 1;
		allocate_info.pSetLayouts = &m_DescriptorSetLayout;
		allocate_info.pNext = nullptr;
		VulkanDebug::CheckVKResult(vkAllocateDescriptorSets(p_OwningDevice->m_LogicalDevice, &allocate_info, &new_set),
			"Vulkan Allocate Descriptor Set Issue!");
		return_val->SetVulkanDescriptorSet(p_OwningDevice, new_set);
		return return_val;
	}
	void VulkanDescriptorSetLayout::BuildLayout()
	{
		if (m_DescriptorSetLayout == VK_NULL_HANDLE)
		{
			std::vector<VkDescriptorSetLayoutBinding> bindings(m_Bindings.size());
			for (size_t binding_id = 0; binding_id < m_Bindings.size(); ++binding_id)
			{
				bindings[binding_id].binding = m_Bindings[binding_id].m_BindingPoint;
				bindings[binding_id].descriptorCount = m_Bindings[binding_id].m_Num;
				bindings[binding_id].descriptorType = TranslateShaderResourceTypeToVulkan(m_Bindings[binding_id].m_ResourceType);
				bindings[binding_id].stageFlags = 0;
				for (auto shader_type : m_Bindings[binding_id].m_ShaderTypes)
				{
					bindings[binding_id].stageFlags |= TranslateShaderTypeToVulkan(shader_type);
				}
			}
			VkDescriptorSetLayoutCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			create_info.bindingCount = static_cast<uint32_t>(bindings.size());
			create_info.pBindings = bindings.data();
			create_info.flags = 0;
			create_info.pNext = nullptr;
			VulkanDebug::CheckVKResult(vkCreateDescriptorSetLayout(p_OwningDevice->m_LogicalDevice, &create_info, VULKAN_ALLOCATOR_POINTER, &m_DescriptorSetLayout), 
				"Vulkan Create Descriptor Set Layout Issue!");
		}
	}
	void VulkanDescriptorSetLayout::Dispose()
	{
		if (m_DescriptorSetLayout != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorSetLayout(p_OwningDevice->m_LogicalDevice, m_DescriptorSetLayout, VULKAN_ALLOCATOR_POINTER);
			m_DescriptorSetLayout = VK_NULL_HANDLE;
		}
		p_OwningDevice->HandleDisposedDescriptorSetLayout(this);
	}

}