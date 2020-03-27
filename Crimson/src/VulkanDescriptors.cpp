#include <headers/VulkanDescriptors.h>
#include <headers/VulkanDebugLog.h>
#include <headers/VulkanTranslator.h>
#include <headers/VulkanBuffer.h>
#include <headers/VulkanAccelerationStructure.h>

namespace Crimson
{
	VulkanDescriptorSet::VulkanDescriptorSet() : 
		p_OwningDevice(nullptr),
		m_DescriptorSet(VK_NULL_HANDLE)
	{}
	/*void VulkanDescriptorSet::WriteDescriptorSetBuffer(uint32_t binding_point, EBufferUniformType uniform_type, PGPUBuffer buffer, uint64_t buffer_offset, uint64_t write_size)
	{
		VulkanBufferObject* p_vulkan_buffer = static_cast<VulkanBufferObject*>(buffer);
		VkDescriptorBufferInfo new_buffer_write_info{};
		new_buffer_write_info.buffer = p_vulkan_buffer->m_Buffer;
		new_buffer_write_info.offset = buffer_offset;
		new_buffer_write_info.range = write_size;
		m_BufferWriteCache[static_cast<size_t>(uniform_type)].push_back(new_buffer_write_info);
	}*/
	void VulkanDescriptorSet::WriteDescriptorSetBuffers(uint32_t binding_point, std::vector<PGPUBuffer> const& buffers, std::vector<BufferRange> const& buffer_ranges, uint32_t start_array_id)
	{
		m_BufferWriteInfoCache.push_back(std::vector<VkDescriptorBufferInfo>(buffers.size()));
		std::vector<VkDescriptorBufferInfo> &new_buffer_infos = *(m_BufferWriteInfoCache.rbegin());
		for (uint32_t id = 0; id < buffers.size(); ++id)
		{
			VulkanBufferObject* p_vulkan_buffer = static_cast<VulkanBufferObject*>(buffers[id]);
			auto& itr_buffer_info = new_buffer_infos[id];
			itr_buffer_info.buffer = p_vulkan_buffer->m_Buffer;
			itr_buffer_info.offset = buffer_ranges[id].m_Offset;
			itr_buffer_info.range = buffer_ranges[id].m_Size;
			m_ReferencedBuffers[m_ResourceReference[binding_point]][start_array_id + id] = p_vulkan_buffer;
		}
		VkWriteDescriptorSet new_write{};
		new_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		new_write.descriptorCount = new_buffer_infos.size();
		//TODO: Change To Translation Function
		new_write.descriptorType = p_OwningSetLayout->m_Bindings[binding_point].m_ResourceType == EShaderResourceType::E_SHADER_UNIFORM_BUFFER ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		new_write.dstArrayElement = start_array_id;
		new_write.dstBinding = binding_point;
		new_write.dstSet = m_DescriptorSet;
		new_write.pBufferInfo = new_buffer_infos.data();
		m_WriteCache.push_back(new_write);
	}
	void VulkanDescriptorSet::WriteDescriptorSetImage(uint32_t binding_point, PGPUImage image, EFilterMode filter_mode, EAddrMode addr_mode, EViewAsType view_as, uint32_t array_id)
	{
		m_ImageWriteInfo.push_back({});
		VkDescriptorImageInfo &new_image_info = *(m_ImageWriteInfo.rbegin());
		VulkanImageObject* p_vulkan_image = static_cast<VulkanImageObject*>(image);
		new_image_info.imageLayout = 
			DetermineLayout(p_vulkan_image->GetDefaultViewAsType(), p_OwningSetLayout->m_Bindings[binding_point].m_ResourceType);
		new_image_info.imageView = p_vulkan_image->GetView(view_as);
		new_image_info.sampler = p_vulkan_image->GetSampler(filter_mode, addr_mode);

		VkWriteDescriptorSet new_write{};
		new_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		new_write.descriptorCount = 1;
		//TODO: Change To Translation Function
		new_write.descriptorType = p_OwningSetLayout->m_Bindings[binding_point].m_ResourceType == EShaderResourceType::E_SHADER_IMAGE_SAMPLER ? VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER : VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		new_write.dstArrayElement = array_id;
		new_write.dstBinding = binding_point;
		new_write.dstSet = m_DescriptorSet;
		new_write.pImageInfo = &new_image_info;
		m_WriteCache.push_back(new_write);
		m_ReferencedImages[m_ResourceReference[binding_point]][array_id] = p_vulkan_image;
	}
	void VulkanDescriptorSet::WriteDescriptorSetAccelStructuresNV(uint32_t binding_point, std::vector<PAccelerationStructure> const& structures)
	{
		m_AccelStructureListCache.push_back(std::vector<VkAccelerationStructureNV>{structures.size()});
		std::vector<VkAccelerationStructureNV>& nv_structures = *(m_AccelStructureListCache.rbegin());
		for (size_t id = 0; id < structures.size(); ++id)
		{
			nv_structures[id] = static_cast<VulkanAccelerationStructure*>(structures[id])->m_Structure;
		}
		m_AccelStructWriteInfoCache.push_back({});
		VkWriteDescriptorSetAccelerationStructureNV &new_accel_info = *(m_AccelStructWriteInfoCache.rbegin());
		new_accel_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV;
		new_accel_info.accelerationStructureCount = static_cast<uint32_t>(nv_structures.size());
		new_accel_info.pAccelerationStructures = nv_structures.data();

		VkWriteDescriptorSet new_write{};
		new_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		new_write.pNext = &new_accel_info;
		new_write.dstSet = m_DescriptorSet;
		new_write.dstBinding = binding_point;
		new_write.descriptorCount = new_accel_info.accelerationStructureCount;
		new_write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
		m_WriteCache.push_back(new_write);
	}
	void VulkanDescriptorSet::EndWriteDescriptorSet()
	{
		if (m_WriteCache.empty()) { return; }
		vkUpdateDescriptorSets(p_OwningDevice->m_LogicalDevice, m_WriteCache.size(), m_WriteCache.data(), 0, nullptr);
		m_WriteCache.clear();
		m_BufferWriteInfoCache.clear();
		m_ImageWriteInfo.clear();
		m_AccelStructWriteInfoCache.clear();
		m_AccelStructureListCache.clear();
	}
	void VulkanDescriptorSet::SetVulkanDescriptorSet(VulkanGPUDevice* device, VkDescriptorSet set, VulkanDescriptorSetLayout* p_layout)
	{
		p_OwningDevice = device;
		m_DescriptorSet = set;
		p_OwningSetLayout = p_layout;
		m_ResourceReference.reserve(p_layout->m_Bindings.size());
		for (auto& binding : p_layout->m_Bindings)
		{
			if (IsBufferResourceType(binding.m_ResourceType))
			{
				m_ResourceReference.push_back(m_ReferencedBuffers.size());
				m_ReferencedBuffers.push_back(std::vector<VulkanBufferObject*>(binding.m_Num, nullptr));
			}
			else if(IsImageResourceType(binding.m_ResourceType))
			{
				m_ResourceReference.push_back(m_ReferencedImages.size());
				m_ReferencedImages.push_back(std::vector<VulkanImageObject*>(binding.m_Num, nullptr));
			}
			else
			{
				m_ResourceReference.push_back(UINT32_MAX);
			}
		}
	}

	void VulkanDescriptorSet::CmdBarrierDescriptorSet(VkCommandBuffer cmd_buffer, uint32_t queue_family, uint32_t type, std::map<VulkanImageObject*, VulkanImageLayoutCache>& image_layout_cache)
	{
		uint32_t id = 0;
		for (auto& binding : p_OwningSetLayout->m_Bindings)
		{
			if (IsImageResourceType(binding.m_ResourceType))
			{
				for (auto image : m_ReferencedImages[m_ResourceReference[id]])
				{
					if (!image->IsStaticLayout())
					{
						VkPipelineStageFlags flags = p_OwningSetLayout->GetReferenceStageRanges(id, type);
						VkImageLayout new_layout = TranslateShaderResourceTypeToVulkanImageLayout(binding.m_ResourceType, image->m_Format);
						image->CmdChangeOverallLayout(cmd_buffer, queue_family,
							new_layout,
							flags, flags);

						auto find = image_layout_cache.find(image);
						if (find == image_layout_cache.end())
						{
							//VulkanImageLayoutCache new_cache;
							//new_cache.m_PielineStages = flags;
							//new_cache.m_EndLayout = new_layout;
							image_layout_cache.insert(std::make_pair(image, image->GetCurrentLayoutCache()));
						}
						else
						{
							//find->second.m_PielineStages = flags;
							//find->second.m_EndLayout = new_layout;
							find->second = image->GetCurrentLayoutCache();
						}
					}
				}
			}
			++id;
		}
	}

	inline VkImageLayout VulkanDescriptorSet::DetermineLayout(EViewAsType view_as, EShaderResourceType resource_type)
	{
		if (resource_type == EShaderResourceType::E_SHADER_STORAGE_IMAGE)
		{
			return VK_IMAGE_LAYOUT_GENERAL;
		}
		return view_as == EViewAsType::E_VIEW_AS_COLOR ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
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
		CHECK_VKRESULT(vkAllocateDescriptorSets(p_OwningDevice->m_LogicalDevice, &allocate_info, &new_set),
			"Vulkan Allocate Descriptor Set Issue!");
		return_val->SetVulkanDescriptorSet(p_OwningDevice, new_set, this);
		return return_val;
	}
	void VulkanDescriptorSetLayout::BuildLayout()
	{
		if (m_DescriptorSetLayout == VK_NULL_HANDLE)
		{
			std::vector<VkDescriptorSetLayoutBinding> bindings(m_Bindings.size());
			m_ReferenceRanges.resize(m_Bindings.size());
			for (size_t binding_id = 0; binding_id < m_Bindings.size(); ++binding_id)
			{
				bindings[binding_id].binding = m_Bindings[binding_id].m_BindingPoint;
				bindings[binding_id].descriptorCount = m_Bindings[binding_id].m_Num;
				bindings[binding_id].descriptorType = TranslateShaderResourceTypeToVulkan(m_Bindings[binding_id].m_ResourceType);
				bindings[binding_id].stageFlags = 0;
				for (auto shader_type : m_Bindings[binding_id].m_ShaderTypes)
				{
					bindings[binding_id].stageFlags |= TranslateShaderTypeToVulkan(shader_type);
					VkPipelineStageFlagBits stage_bit = TranslateShaderTypeToVulkanPipelineStage(shader_type);
					if (IsNormalGraphicsShaderGroup(shader_type))
					{
						m_ReferenceRanges[binding_id].m_GraphicsStages |= stage_bit;
					}
					if (IsMeshShaderGroup(shader_type))
					{
						m_ReferenceRanges[binding_id].m_MeshPipelineStages |= stage_bit;
					}
					if (IsRayTracingShaderGroup(shader_type))
					{
						m_ReferenceRanges[binding_id].m_RayTracingStages |= stage_bit;
					}
				}
			}
			VkDescriptorSetLayoutCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			create_info.bindingCount = static_cast<uint32_t>(bindings.size());
			create_info.pBindings = bindings.data();
			create_info.flags = 0;
			create_info.pNext = nullptr;
			CHECK_VKRESULT(vkCreateDescriptorSetLayout(p_OwningDevice->m_LogicalDevice, &create_info, VULKAN_ALLOCATOR_POINTER, &m_DescriptorSetLayout), 
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

	VkPipelineStageFlags VulkanDescriptorSetLayout::GetReferenceStageRanges(uint32_t binding_id, uint32_t type)
	{
		switch (type)
		{
		case 0:
			return m_ReferenceRanges[binding_id].m_GraphicsStages;
		case 1:
			return m_ReferenceRanges[binding_id].m_MeshPipelineStages;
		case 2:
			return m_ReferenceRanges[binding_id].m_RayTracingStages;
		}
		return VkPipelineStageFlags();
	}

}