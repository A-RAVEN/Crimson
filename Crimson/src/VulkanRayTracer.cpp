#include <headers/VulkanRayTracer.h>
#include <headers/VulkanDebugLog.h>
#include <headers/VulkanTranslator.h>
#include <headers/VulkanDescriptors.h>
#include <map>

namespace Crimson
{
	VulkanRayTracer::VulkanRayTracer(VulkanGPUDevice* device) :
		p_OwningDevice(device),
		m_PipelineLayout(VK_NULL_HANDLE),
		m_Pipeline(VK_NULL_HANDLE)
	{}
	void VulkanRayTracer::LoadShaderSource(char const* src_code, size_t src_size, EShaderType shader_type, std::string const& shader_table)
	{
		if (shader_type >= EShaderType::E_SHADER_TYPE_RAYGEN_NV && shader_type <= EShaderType::E_SHADER_TYPE_INTERSECTION_NV) {
			VkShaderModule new_shader_module = VK_NULL_HANDLE;
			VkShaderModuleCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			create_info.pCode = reinterpret_cast<const uint32_t*>(src_code);
			create_info.codeSize = src_size;
			create_info.flags = 0;
			CHECK_VKRESULT(vkCreateShaderModule(p_OwningDevice->m_LogicalDevice, &create_info, VULKAN_ALLOCATOR_POINTER, &new_shader_module),
				"Vulkan Creat Shader Module Issue!");
			m_Shaders.push_back(std::make_pair(new_shader_module, shader_type));
			auto find = m_ShaderTables.find(shader_table);
			if (find != m_ShaderTables.end())
			{
				find->second.push_back(m_Shaders.size() - 1);
			}
			else
			{
				std::vector<size_t> new_list = { m_Shaders.size() - 1 };
				m_ShaderTables.insert(std::make_pair(shader_table, new_list));
			}
		}
	}
	uint64_t VulkanRayTracer::GetShaderTableSize(std::string const& shader_table_name)
	{
		auto find = m_ShaderTables.find(shader_table_name);
		if (find != m_ShaderTables.end())
		{
			return find->second.size() * p_OwningDevice->m_NVExtension.m_RayTracingProperties.shaderGroupHandleSize;
		}
		return 0u;
	}
	void VulkanRayTracer::CopyShaderTable(void* copy_dst, std::string const& shader_table_name)
	{
		auto find = m_ShaderTables.find(shader_table_name);
		if (find != m_ShaderTables.end())
		{
			char* dst = static_cast<char*>(copy_dst);
			for (uint64_t i = 0; i < find->second.size(); ++i)
			{
				uint32_t handle_size = p_OwningDevice->m_NVExtension.m_RayTracingProperties.shaderGroupHandleSize;
				memcpy(dst + i * handle_size, &m_Handles[find->second[i] * handle_size], handle_size);
			}
		}
	}
	void VulkanRayTracer::Build()
	{
		std::vector<VkRayTracingShaderGroupCreateInfoNV> shader_group_create_infos(m_Shaders.size());
		std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create_infos(m_Shaders.size());
		for (uint32_t shader_id = 0; shader_id < m_Shaders.size(); ++shader_id)
		{
			VkRayTracingShaderGroupCreateInfoNV& itr_group_info = shader_group_create_infos[shader_id];
			VkPipelineShaderStageCreateInfo& itr_stage_info = shader_stage_create_infos[shader_id];

			auto& shader_pair = m_Shaders[shader_id];
			itr_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			itr_stage_info.module = shader_pair.first;
			itr_stage_info.pName = "main";
			itr_stage_info.stage = TranslateShaderTypeToVulkan(shader_pair.second);
			itr_stage_info.flags = 0;
			itr_stage_info.pSpecializationInfo = nullptr;
			itr_stage_info.pNext = nullptr;

			itr_group_info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
			itr_group_info.generalShader = itr_group_info.anyHitShader = itr_group_info.closestHitShader = itr_group_info.intersectionShader = VK_SHADER_UNUSED_NV;
			switch (shader_pair.second)
			{
			case EShaderType::E_SHADER_TYPE_RAYGEN_NV:
			case EShaderType::E_SHADER_TYPE_MISS_NV:
				itr_group_info.generalShader = shader_id;
				itr_group_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
				break;
			case EShaderType::E_SHADER_TYPE_CLOSEHIT_NV:
				itr_group_info.closestHitShader = shader_id;
				itr_group_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
				break;
			case EShaderType::E_SHADER_TYPE_ANYHIT_NV:
				itr_group_info.anyHitShader = shader_id;
				itr_group_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
				break;
			case EShaderType::E_SHADER_TYPE_INTERSECTION_NV:
				itr_group_info.intersectionShader = shader_id;
				itr_group_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV;
				break;
			}
		}

		//pipeline layout
		VkPipelineLayout new_pipeline_layout;
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		size_t set_layout_size = m_DescriptorSetLayouts.size();
		std::map<uint32_t, VkDescriptorSetLayout> layout_map;
		for (auto& layout : m_DescriptorSetLayouts)
		{
			VulkanDescriptorSetLayout* vulkan_layout = static_cast<VulkanDescriptorSetLayout*>(layout.second);
			layout_map.insert(std::make_pair(layout.first, vulkan_layout->m_DescriptorSetLayout));
		}
		std::vector<VkDescriptorSetLayout> set_layouts(set_layout_size);
		set_layouts.clear();
		for (auto& itr_layout : layout_map)
		{
			set_layouts.push_back(itr_layout.second);
		}
		pipelineLayoutInfo.setLayoutCount = set_layouts.size();
		pipelineLayoutInfo.pSetLayouts = pipelineLayoutInfo.setLayoutCount > 0 ? set_layouts.data() : nullptr;
		VkPushConstantRange push_constant_range{};
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
		CHECK_VKRESULT(vkCreatePipelineLayout(p_OwningDevice->m_LogicalDevice, &pipelineLayoutInfo, nullptr, &m_PipelineLayout), "Vulkan Pipeline Layout For Ray Tracer Creation Issue!");

		VkRayTracingPipelineCreateInfoNV pipeline_create_info{};
		pipeline_create_info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
		pipeline_create_info.groupCount = shader_group_create_infos.size();
		pipeline_create_info.pGroups = shader_group_create_infos.data();
		pipeline_create_info.stageCount = shader_stage_create_infos.size();
		pipeline_create_info.pStages = shader_stage_create_infos.data();
		pipeline_create_info.maxRecursionDepth = m_MaxRecursionDepth;
		pipeline_create_info.layout = m_PipelineLayout;
		CHECK_VKRESULT(p_OwningDevice->m_NVExtension.vkCreateRayTracingPipelinesNV(p_OwningDevice->m_LogicalDevice, VK_NULL_HANDLE, 1, &pipeline_create_info, VULKAN_ALLOCATOR_POINTER, &m_Pipeline), "Vulkan Create Ray Tracing Pipeline Issue!");

		m_Handles.resize(pipeline_create_info.groupCount * p_OwningDevice->m_NVExtension.m_RayTracingProperties.shaderGroupHandleSize);
		CHECK_VKRESULT(p_OwningDevice->m_NVExtension.vkGetRayTracingShaderGroupHandlesNV(p_OwningDevice->m_LogicalDevice, m_Pipeline, 0, pipeline_create_info.groupCount,
			m_Handles.size(), m_Handles.data()), "Vulkan Get Ray Tracing Shader Group Handle Issue!");
	}
	void VulkanRayTracer::Dispose()
	{
		DestroyPipeline();
		DestroyShaderModules();
		p_OwningDevice->HandleDisposedRayTracer(this);
	}
	void VulkanRayTracer::DestroyPipeline()
	{
		if (m_Pipeline != VK_NULL_HANDLE)
		{
			vkDestroyPipeline(p_OwningDevice->m_LogicalDevice, m_Pipeline, VULKAN_ALLOCATOR_POINTER);
			m_Pipeline = VK_NULL_HANDLE;
		}
		if (m_PipelineLayout != VK_NULL_HANDLE)
		{
			vkDestroyPipelineLayout(p_OwningDevice->m_LogicalDevice, m_PipelineLayout, VULKAN_ALLOCATOR_POINTER);
			m_PipelineLayout = VK_NULL_HANDLE;
		}
	}
	void VulkanRayTracer::DestroyShaderModules()
	{
		for (auto& pair : m_Shaders)
		{
			vkDestroyShaderModule(p_OwningDevice->m_LogicalDevice, pair.first, VULKAN_ALLOCATOR_POINTER);
		}
		m_Shaders.clear();
	}
}