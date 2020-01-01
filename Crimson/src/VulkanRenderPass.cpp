#include <headers/VulkanRenderPass.h>
#include <headers/VulkanTranslator.h>
#include <headers/VulkanDebugLog.h>
#include <headers/VulkanPipeline.h>
#include <headers/VulkanDescriptors.h>
#include <set>
#include <map>

namespace Crimson
{
	struct SubpassDependencyTracker
	{
		uint32_t m_PreviouseSubpass;
		uint32_t m_CurrentSubpass;
		std::vector<VkImageLayout> m_PreviouseLayouts;
		std::vector<VkImageLayout> m_CurrentSubpassLayouts;
		VkAccessFlags m_PreviouseAccessFlags;
		VkAccessFlags m_CurrentAccessFlags;
		VkPipelineStageFlags m_PreviouseStageFlags;
		VkPipelineStageFlags m_CurrentStageFlags;
		SubpassDependencyTracker(uint32_t attachment_num) :
			m_PreviouseSubpass(VK_SUBPASS_EXTERNAL),
			m_CurrentSubpass(0),
			m_PreviouseLayouts(attachment_num),
			m_CurrentSubpassLayouts(attachment_num),
			m_PreviouseAccessFlags(0),
			m_CurrentAccessFlags(0),
			m_PreviouseStageFlags(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT),
			m_CurrentStageFlags(0)
		{}
		void NextSubpass()
		{
			std::copy(m_PreviouseLayouts.begin(), m_PreviouseLayouts.end(), m_CurrentSubpassLayouts.begin());
			m_PreviouseAccessFlags = m_CurrentAccessFlags;
			m_CurrentAccessFlags = 0;
			if (m_CurrentStageFlags != 0) {
				m_PreviouseStageFlags = m_CurrentStageFlags;
				m_CurrentStageFlags = 0;
			}
			m_PreviouseSubpass = m_CurrentSubpass;
			m_CurrentSubpass++;
		}
		void PushSubpassDependencies(std::vector<VkSubpassDependency>& dependencies)
		{
			for (size_t layout_id = 0; layout_id < m_CurrentSubpassLayouts.size(); ++layout_id)
			{
				if (m_CurrentSubpassLayouts[layout_id] != m_PreviouseLayouts[layout_id])
				{
					VkSubpassDependency new_dependency{};
					new_dependency.srcSubpass = m_PreviouseSubpass;
					new_dependency.dstSubpass = m_CurrentSubpass;
					new_dependency.srcStageMask = m_PreviouseStageFlags;
					new_dependency.dstStageMask = m_CurrentStageFlags;
					new_dependency.srcAccessMask = m_PreviouseAccessFlags;
					new_dependency.dstAccessMask = m_CurrentAccessFlags;
					//TODO: Need More Modifications
					new_dependency.dependencyFlags = 0;
					dependencies.push_back(new_dependency);
				}
			}
		}
	};

	VulkanRenderPass::VulkanRenderPass(VulkanGPUDevice* owning_device) : p_OwningDevice(owning_device), m_RenderPass(VK_NULL_HANDLE)
	{}

	void VulkanRenderPass::BuildRenderPass()
	{
		if (p_OwningDevice != nullptr && m_RenderPass == VK_NULL_HANDLE)
		{
			m_VulkanSubpassInfos.clear();
			m_VulkanAttachmentInfos.clear();
			m_VulkanSubpassInfos.resize(m_Subpasses.size());
			std::vector<VkAttachmentDescription> vk_attachments;
			SubpassDependencyTracker dependency_tracker(static_cast<uint32_t>(m_Attachments.size()));
			for (uint32_t attachment_id = 0; attachment_id < m_Attachments.size(); ++attachment_id)
			{
				bool is_color_format = IsColorFormat(m_Attachments[attachment_id].m_Format);
				if (is_color_format)
				{
					dependency_tracker.m_CurrentSubpassLayouts[attachment_id] = 
						dependency_tracker.m_PreviouseLayouts[attachment_id] = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				}
				else
				{
					dependency_tracker.m_CurrentSubpassLayouts[attachment_id] = 
						dependency_tracker.m_PreviouseLayouts[attachment_id] = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				}
			}
			std::vector<VkSubpassDescription> vk_subpasses(m_Subpasses.size());
			std::vector<VkSubpassDependency> vk_subpass_dependencies;
			std::vector<std::vector<VkAttachmentReference>> vk_color_attachment_references(m_Subpasses.size());
			std::vector<std::vector<VkAttachmentReference>> vk_input_attachment_references(m_Subpasses.size());
			std::vector<std::vector<VkAttachmentReference>> vk_depth_stencil_attachment_refrences(m_Subpasses.size());
			{
				//solve for true attachments for depth stencil texture
				std::map<int32_t, uint32_t> output_set;
				std::map<int32_t, uint32_t> input_set;
				std::map<int32_t, uint32_t> depth_input_view_set;
				std::map<int32_t, uint32_t> stencil_input_view_set;
				std::map<int32_t, uint32_t> depth_stencil_view_set;
				for (uint32_t subpass_id = 0; subpass_id < m_Subpasses.size(); ++subpass_id)
				{
					std::map<int32_t, uint32_t> subpass_depth_input_set;
					std::map<int32_t, uint32_t> subpass_stencil_input_set;
					std::vector<VkAttachmentReference>& r_subpass_output_attachment_refs = vk_color_attachment_references[subpass_id];
					std::vector<VkAttachmentReference>& r_subpass_input_attachment_refs = vk_input_attachment_references[subpass_id];
					std::vector<VkAttachmentReference>& r_subpass_depth_stencil_attachment_refs = vk_depth_stencil_attachment_refrences[subpass_id];
					//color outputs
					for (int32_t attachment_ref : m_Subpasses[subpass_id].m_OutputAttachments)
					{
						VkAttachmentReference new_reference{};
						new_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						dependency_tracker.m_CurrentSubpassLayouts[attachment_ref] = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						dependency_tracker.m_CurrentStageFlags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
						dependency_tracker.m_CurrentAccessFlags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
						auto find = output_set.find(attachment_ref);
						if (find != output_set.end())
						{
							new_reference.attachment = find->second;
						}
						else
						{
							{
								RenderPassAttachment& ref_attachment = m_Attachments[attachment_ref];
								//push back new attachment description
								VkAttachmentDescription new_description{};
								new_description.loadOp = ref_attachment.m_ClearType == EAttachmentClearType::E_ATTACHMENT_NOT_CLEAR ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
								new_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
								new_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
								new_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
								vk_attachments.push_back(new_description);
							}
							VulkanAttachmentInfo new_info{};
							new_info.m_ImageReference = attachment_ref;
							m_VulkanAttachmentInfos.push_back(new_info);
							uint32_t new_attachment_index = static_cast<uint32_t>(m_VulkanAttachmentInfos.size() - 1);
							output_set.insert(std::make_pair(attachment_ref, new_attachment_index));
							new_reference.attachment = new_attachment_index;
						}
						r_subpass_output_attachment_refs.push_back(new_reference);
					}
					//color inputs
					for (int32_t attachment_ref : m_Subpasses[subpass_id].m_InputAttachments)
					{
						VkAttachmentReference new_reference{};
						new_reference.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						dependency_tracker.m_CurrentSubpassLayouts[attachment_ref] = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						dependency_tracker.m_CurrentStageFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
						dependency_tracker.m_CurrentAccessFlags |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
						auto find = input_set.find(attachment_ref);
						if (find != input_set.end())
						{
							new_reference.attachment = find->second;
						}
						else
						{
							{
								//push back new attachment description
								VkAttachmentDescription new_description{};
								new_description.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
								new_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
								new_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
								new_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
								vk_attachments.push_back(new_description);
							}
							VulkanAttachmentInfo new_info{};
							new_info.m_ImageReference = attachment_ref;
							m_VulkanAttachmentInfos.push_back(new_info);
							uint32_t new_attachment_index = static_cast<uint32_t>(m_VulkanAttachmentInfos.size() - 1);
							input_set.insert(std::make_pair(attachment_ref, new_attachment_index));
							new_reference.attachment = new_attachment_index;
						}
						r_subpass_input_attachment_refs.push_back(new_reference);
					}
					//depth inputs
					for (int32_t attachment_ref : m_Subpasses[subpass_id].m_DepthInputAttachments)
					{
						VkAttachmentReference new_reference{};
						new_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
						dependency_tracker.m_CurrentSubpassLayouts[attachment_ref] = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
						dependency_tracker.m_CurrentStageFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
						dependency_tracker.m_CurrentAccessFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
						auto find = depth_input_view_set.find(attachment_ref);
						if (find != depth_input_view_set.end())
						{
							new_reference.attachment = find->second;
						}
						else
						{
							{
								//push back new attachment description
								VkAttachmentDescription new_description{};
								new_description.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
								new_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
								new_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
								new_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
								vk_attachments.push_back(new_description);
							}
							VulkanAttachmentInfo new_info{};
							new_info.m_ImageReference = attachment_ref;
							m_VulkanAttachmentInfos.push_back(new_info);
							uint32_t new_attachment_index = static_cast<uint32_t>(m_VulkanAttachmentInfos.size() - 1);
							depth_input_view_set.insert(std::make_pair(attachment_ref, new_attachment_index));
							new_reference.attachment = new_attachment_index;
						}
						r_subpass_input_attachment_refs.push_back(new_reference);
						subpass_depth_input_set.insert(std::make_pair(attachment_ref, static_cast<uint32_t>(r_subpass_input_attachment_refs.size() - 1)));
					}
					//stencil inputs
					for (int32_t attachment_ref : m_Subpasses[subpass_id].m_StencilInputAttachments)
					{
						VkAttachmentReference new_reference{};
						new_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
						dependency_tracker.m_CurrentSubpassLayouts[attachment_ref] = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
						dependency_tracker.m_CurrentStageFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
						dependency_tracker.m_CurrentAccessFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
						auto find = stencil_input_view_set.find(attachment_ref);
						if (find != stencil_input_view_set.end())
						{
							new_reference.attachment = find->second;
						}
						else
						{
							{
								//push back new attachment description
								VkAttachmentDescription new_description{};
								new_description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
								new_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
								new_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
								new_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
								vk_attachments.push_back(new_description);
							}
							VulkanAttachmentInfo new_info{};
							new_info.m_ImageReference = attachment_ref;
							m_VulkanAttachmentInfos.push_back(new_info);
							uint32_t new_attachment_index = static_cast<uint32_t>(m_VulkanAttachmentInfos.size() - 1);
							stencil_input_view_set.insert(std::make_pair(attachment_ref, new_attachment_index));
							new_reference.attachment = new_attachment_index;
						}
						r_subpass_input_attachment_refs.push_back(new_reference);
						subpass_stencil_input_set.insert(std::make_pair(attachment_ref, static_cast<uint32_t>(r_subpass_input_attachment_refs.size())));
					}
					//depth stencil attachments
					if (m_Subpasses[subpass_id].m_DepthStencilAttachment >= 0)
					{
						VkAttachmentReference new_reference{};
						new_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
						dependency_tracker.m_CurrentSubpassLayouts[m_Subpasses[subpass_id].m_DepthStencilAttachment] = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
						dependency_tracker.m_CurrentStageFlags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
						auto find = depth_stencil_view_set.find(m_Subpasses[subpass_id].m_DepthStencilAttachment);
						if (find != depth_stencil_view_set.end())
						{
							new_reference.attachment = find->second;
						}
						else
						{
							{
								RenderPassAttachment& ref_attachment = m_Attachments[m_Subpasses[subpass_id].m_DepthStencilAttachment];
								//push back new attachment description
								VkAttachmentDescription new_description{};
								new_description.loadOp = ref_attachment.m_ClearType == EAttachmentClearType::E_ATTACHMENT_NOT_CLEAR ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
								new_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
								new_description.stencilLoadOp = ref_attachment.m_ClearType == EAttachmentClearType::E_ATTACHMENT_NOT_CLEAR ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
								new_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
								vk_attachments.push_back(new_description);
							}
							VulkanAttachmentInfo new_info{};
							new_info.m_ImageReference = m_Subpasses[subpass_id].m_DepthStencilAttachment;
							m_VulkanAttachmentInfos.push_back(new_info);
							uint32_t new_attachment_index = static_cast<uint32_t>(m_VulkanAttachmentInfos.size() - 1);
							depth_stencil_view_set.insert(std::make_pair(m_Subpasses[subpass_id].m_DepthStencilAttachment, new_attachment_index));
							new_reference.attachment = new_attachment_index;
						}
						{
							//check if depth input and stencil input of current depth / stencil attachment is used in current subpass
							auto depth_input_find = subpass_depth_input_set.find(m_Subpasses[subpass_id].m_DepthStencilAttachment);
							auto stencil_input_find = subpass_stencil_input_set.find(m_Subpasses[subpass_id].m_DepthStencilAttachment);
							bool depth_readonly = depth_input_find != subpass_depth_input_set.end();
							bool stencil_readonly = stencil_input_find != subpass_stencil_input_set.end();
							//if depth stencil buffer is readonly, or both depth stencil are treated as input attachment, then both channels should be readonly
							if (m_Subpasses[subpass_id].b_DepthStencilReadOnly || (depth_readonly && stencil_readonly))
							{
								new_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
								dependency_tracker.m_CurrentSubpassLayouts[m_Subpasses[subpass_id].m_DepthStencilAttachment] = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
								dependency_tracker.m_CurrentAccessFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
							}
							//if depth is treated as input attachment, then depth channel should be readonly
							else if(depth_readonly)
							{
								new_reference.layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
								r_subpass_input_attachment_refs[depth_input_find->second].layout = new_reference.layout;
								dependency_tracker.m_CurrentSubpassLayouts[m_Subpasses[subpass_id].m_DepthStencilAttachment] = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
								dependency_tracker.m_CurrentAccessFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
							}
							//if stencil is treated as input attachment, then stencil channel should be readonly
							else if (stencil_readonly)
							{
								new_reference.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
								r_subpass_input_attachment_refs[stencil_input_find->second].layout = new_reference.layout;
								dependency_tracker.m_CurrentSubpassLayouts[m_Subpasses[subpass_id].m_DepthStencilAttachment] = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
								dependency_tracker.m_CurrentAccessFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
							}
							else
							{
								dependency_tracker.m_CurrentAccessFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
							}
						}
						r_subpass_depth_stencil_attachment_refs.push_back(new_reference);
					}
					//push back subpass dependency
					dependency_tracker.PushSubpassDependencies(vk_subpass_dependencies);
					dependency_tracker.NextSubpass();
					VkSubpassDescription& current_subpass = vk_subpasses[subpass_id];
					current_subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; //Default Usage For Now
					current_subpass.colorAttachmentCount = static_cast<uint32_t>(r_subpass_output_attachment_refs.size());
					current_subpass.pColorAttachments = current_subpass.colorAttachmentCount > 0 ? r_subpass_output_attachment_refs.data() : nullptr;
					current_subpass.inputAttachmentCount = static_cast<uint32_t>(r_subpass_input_attachment_refs.size());
					current_subpass.pInputAttachments = r_subpass_input_attachment_refs.size() > 0 ? r_subpass_input_attachment_refs.data() : nullptr;
					current_subpass.pDepthStencilAttachment = r_subpass_depth_stencil_attachment_refs.size() > 0 ? r_subpass_depth_stencil_attachment_refs.data() : nullptr;
					//TODO: Need Further Improvements
					current_subpass.pPreserveAttachments = nullptr;
					current_subpass.preserveAttachmentCount = 0;
					current_subpass.pResolveAttachments = nullptr;	//should be same number as color attachments
					//Optional
					current_subpass.flags = 0;

					//check and create descriptor set for input attachments
					if(r_subpass_input_attachment_refs.size() > 0)
					{
						std::vector<VkDescriptorSetLayoutBinding> bindings(r_subpass_input_attachment_refs.size());
						for (uint32_t binding_id = 0; binding_id < bindings.size(); ++binding_id)
						{
							bindings[binding_id].binding = binding_id;
							bindings[binding_id].descriptorCount = 1;
							bindings[binding_id].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
							bindings[binding_id].stageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
							bindings[binding_id].pImmutableSamplers = nullptr;
						}
						VkDescriptorSetLayoutCreateInfo set_layout_create_info{};
						set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
						set_layout_create_info.flags = 0;
						set_layout_create_info.bindingCount = static_cast<uint32_t>(r_subpass_input_attachment_refs.size());
						set_layout_create_info.pBindings = bindings.data();
						set_layout_create_info.pNext = nullptr;
						VulkanDebug::CheckVKResult(vkCreateDescriptorSetLayout(p_OwningDevice->m_LogicalDevice, &set_layout_create_info, VULKAN_ALLOCATOR_POINTER, &m_VulkanSubpassInfos[subpass_id].m_InputLayout),
							"Vulkan Subpass Attachment Input Descriptor Set Layout Creation Issue!");
					}
				}
			}
			m_ClearValues.resize(m_VulkanAttachmentInfos.size());
			{
				for (size_t attachment_id = 0; attachment_id < vk_attachments.size(); ++attachment_id)
				{
					VkAttachmentDescription& current_attachment = vk_attachments[attachment_id];
					RenderPassAttachment& ref_attachment = m_Attachments[m_VulkanAttachmentInfos[attachment_id].m_ImageReference];
					current_attachment.format = TranslateImageFormatToVulkan(ref_attachment.m_Format);
					current_attachment.flags = 0;
					if (IsColorFormat(ref_attachment.m_Format))
					{
						current_attachment.initialLayout = current_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					}
					else
					{
						current_attachment.initialLayout = current_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					}
					current_attachment.samples = TranslateSampleCountToVulkan(ref_attachment.m_SampleCount);
				}
			}
			VkRenderPassCreateInfo create_info = {};
			create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			create_info.attachmentCount = static_cast<uint32_t>(m_VulkanAttachmentInfos.size());
			create_info.pAttachments = vk_attachments.data();
			create_info.subpassCount = static_cast<uint32_t>(m_Subpasses.size());
			create_info.pSubpasses = vk_subpasses.data();
			create_info.dependencyCount = static_cast<uint32_t>(vk_subpass_dependencies.size());
			create_info.pDependencies = vk_subpass_dependencies.size() > 0 ? vk_subpass_dependencies.data() : nullptr;

			VulkanDebug::CheckVKResult(vkCreateRenderPass(p_OwningDevice->m_LogicalDevice, &create_info, VULKAN_ALLOCATOR_POINTER, &m_RenderPass), "Vulkan Create RenderPass Issue!");
		}
	}
	void VulkanRenderPass::InstanciatePipeline(GraphicsPipeline* pipeline, uint32_t subpass)
	{
		BuildRenderPass();

		VulkanGraphicsPipeline const* vulkan_pipeline = static_cast<VulkanGraphicsPipeline const*>(pipeline);
		size_t shader_count = vulkan_pipeline->m_Shaders.size();
		std::vector<VkPipelineShaderStageCreateInfo> shader_infos(shader_count);
		for (size_t id = 0; id < shader_count; ++id)
		{
			shader_infos[id].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shader_infos[id].module = vulkan_pipeline->m_Shaders[id].first;
			shader_infos[id].stage = TranslateShaderTypeToVulkan(vulkan_pipeline->m_Shaders[id].second);
			shader_infos[id].pName = "main";
			//TODO: Need explorations
			shader_infos[id].flags = 0;
			shader_infos[id].pNext = nullptr;
			shader_infos[id].pSpecializationInfo = nullptr;
		}
		//get vertex inputs
		size_t vertex_inpute_size = vulkan_pipeline->m_VertexInputs.size();
		std::vector<VkVertexInputBindingDescription> vk_vi_descriptions(vertex_inpute_size);
		std::vector<VkVertexInputAttributeDescription> vk_ad_descriptions;
		uint32_t attribute_location = 0;
		for (uint32_t vertex_input_id = 0; vertex_input_id < vertex_inpute_size; ++vertex_input_id)
		{
			auto& a_data = vulkan_pipeline->m_VertexInputs[vertex_input_id].m_DataTypes;
			size_t ad_size = a_data.size();

			uint32_t acc_stride = 0;
			for (size_t ad_id = 0; ad_id < ad_size; ++ad_id)
			{
				auto& vertex_inpuf_data = TranslateDataTypeToVulkanVertexInputDataTypeInfo(a_data[ad_id]);
				VkVertexInputAttributeDescription new_description = {};
				new_description.binding = vertex_input_id;
				new_description.format = vertex_inpuf_data.m_Format;

				uint32_t location_size = vertex_inpuf_data.m_LocationOccupation;
				uint32_t data_type_size = vertex_inpuf_data.m_Size;
				for (uint32_t i = 0; i < location_size; ++i)
				{
					new_description.location = attribute_location;
					new_description.offset = acc_stride;
					vk_ad_descriptions.push_back(new_description);
					acc_stride += data_type_size;
					++attribute_location;
				}
			}
			vk_vi_descriptions[vertex_input_id].binding = vertex_input_id;
			vk_vi_descriptions[vertex_input_id].stride = acc_stride;
			vk_vi_descriptions[vertex_input_id].inputRate = 
				vulkan_pipeline->m_VertexInputs[vertex_input_id].m_VertexInputMode == 
				EVertexInputMode::E_VERTEX_INPUT_PER_VERTEX ? 
				VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;
		}

		//vertex input info, empty for now
		VkPipelineVertexInputStateCreateInfo vertex_input = {};
		vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input.vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_inpute_size);
		vertex_input.pVertexBindingDescriptions = vk_vi_descriptions.data(); // Optional
		vertex_input.vertexAttributeDescriptionCount = static_cast<uint32_t>(vk_ad_descriptions.size());
		vertex_input.pVertexAttributeDescriptions = vk_ad_descriptions.data(); // Optional

		//input assemply stage
		VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
		input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly.topology = TranslateTopologyToVulkan(vulkan_pipeline->m_Topology);
		input_assembly.primitiveRestartEnable = VK_FALSE;

		////viewport and sissors
		//VkViewport viewport = {};
		//viewport.x = 0.0f;
		//viewport.y = 0.0f;
		//viewport.width = 512.0f;
		//viewport.height = 512.0f;
		//viewport.minDepth = 0.0f;
		//viewport.maxDepth = 1.0f;

		//VkRect2D scissor = {};
		//scissor.offset = { 0, 0 };
		//scissor.extent = { 512, 512 };

		VkPipelineViewportStateCreateInfo viewport_state = {};
		viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state.viewportCount = 0;// 1;
		viewport_state.pViewports = nullptr;// &viewport;
		viewport_state.scissorCount = 0;// 1;
		viewport_state.pScissors = nullptr;// &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_TRUE;
		rasterizer.polygonMode = TranslatePolygonModeToVulkan(vulkan_pipeline->m_PolygonMode);
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = TranslateCullModeToVulkan(vulkan_pipeline->m_CullMode);
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;//wait and see VK_FRONT_FACE_CLOCKWISE
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		//multisampling, currently disabled
		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		if (vulkan_pipeline->m_MultiSampleShadingNum > 0)
		{
			multisampling.sampleShadingEnable = VK_TRUE;
			multisampling.rasterizationSamples = TranslateSampleCountToVulkan(vulkan_pipeline->m_MultiSampleShadingNum);
			multisampling.minSampleShading = 1.0f / vulkan_pipeline->m_MultiSampleShadingNum;
			multisampling.pSampleMask = nullptr; // Optional
			multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
			multisampling.alphaToOneEnable = VK_FALSE;
		}
		else
		{
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisampling.minSampleShading = 1.0f; // Optional
			multisampling.pSampleMask = nullptr; // Optional
			multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
			multisampling.alphaToOneEnable = VK_FALSE; // Optional
		}

		//depth and stencil, currently null for stencil test
		VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
		depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil.depthCompareOp = TranslateCompareModeToVulkan(vulkan_pipeline->m_DepthCompareMode);
		//assert(vulkan_pipeline->m_DepthRule < EDepthTestRule::E_DEPTH_TEST_MAX);
		switch (vulkan_pipeline->m_DepthRule)
		{
		case EDepthTestRule::E_DEPTH_TEST_ENABLED:
			depth_stencil.depthTestEnable = VK_TRUE;
			depth_stencil.depthWriteEnable = VK_TRUE;
			depth_stencil.depthBoundsTestEnable = VK_FALSE;
			depth_stencil.minDepthBounds = 0.0f; // Optional
			depth_stencil.maxDepthBounds = 1.0f; // Optional
			break;
		case EDepthTestRule::E_DEPTH_TEST_DISABLED:
			depth_stencil.depthTestEnable = VK_FALSE;
			depth_stencil.depthWriteEnable = VK_FALSE;
			depth_stencil.depthBoundsTestEnable = VK_FALSE;
			depth_stencil.minDepthBounds = 0.0f; // Optional
			depth_stencil.maxDepthBounds = 1.0f; // Optional
			break;
		case EDepthTestRule::E_DEPTH_TEST_READ_ONLY:
			depth_stencil.depthTestEnable = VK_TRUE;
			depth_stencil.depthWriteEnable = VK_FALSE;
			depth_stencil.depthBoundsTestEnable = VK_FALSE;
			depth_stencil.minDepthBounds = 0.0f; // Optional
			depth_stencil.maxDepthBounds = 1.0f; // Optional
			break;
		default:
			break;
		}


		//assert(vulkan_pipeline->m_StencilRule < EStencilRule::E_STENCIL_MAX);
		switch (vulkan_pipeline->m_StencilRule)
		{
		case EStencilRule::E_STENCIL_DISABLED:
			depth_stencil.stencilTestEnable = VK_FALSE;
			depth_stencil.front = {};
			depth_stencil.back = {};
			break;
		case EStencilRule::E_STENCIL_WRITE:
			depth_stencil.stencilTestEnable = VK_TRUE;
			depth_stencil.front.compareOp = VK_COMPARE_OP_ALWAYS;
			depth_stencil.front.passOp = VK_STENCIL_OP_REPLACE;
			depth_stencil.front.failOp = VK_STENCIL_OP_KEEP;
			depth_stencil.front.depthFailOp = VK_STENCIL_OP_KEEP;
			depth_stencil.front.compareMask = 0xff;
			depth_stencil.front.writeMask = 0xff;
			depth_stencil.front.reference = 1;
			depth_stencil.back = depth_stencil.front;
			break;
		case EStencilRule::E_STENCIL_TEST:
			depth_stencil.stencilTestEnable = VK_TRUE;
			depth_stencil.front.passOp = VK_STENCIL_OP_KEEP;
			depth_stencil.front.failOp = VK_STENCIL_OP_KEEP;
			depth_stencil.front.depthFailOp = VK_STENCIL_OP_KEEP;
			depth_stencil.front.compareOp = VK_COMPARE_OP_EQUAL;
			depth_stencil.front.compareMask = 0xff;
			depth_stencil.front.writeMask = 0x0;
			depth_stencil.front.reference = 1;
			depth_stencil.back = depth_stencil.front;
			break;
		case EStencilRule::E_STENCIL_INVTEST:
			depth_stencil.stencilTestEnable = VK_TRUE;
			depth_stencil.front.passOp = VK_STENCIL_OP_KEEP;
			depth_stencil.front.failOp = VK_STENCIL_OP_KEEP;
			depth_stencil.front.depthFailOp = VK_STENCIL_OP_KEEP;
			depth_stencil.front.compareOp = VK_COMPARE_OP_NOT_EQUAL;
			depth_stencil.front.compareMask = 0xff;
			depth_stencil.front.writeMask = 0x0;
			depth_stencil.front.reference = 1;
			depth_stencil.back = depth_stencil.front;
			break;
		}

		VkPipelineColorBlendAttachmentState color_blend_attachment = {};
		color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		color_blend_attachment.blendEnable = VK_FALSE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

		uint32_t blend_attachments_size = m_Subpasses[subpass].m_OutputAttachments.size();
		std::vector<VkPipelineColorBlendAttachmentState>blend_attachments(blend_attachments_size);
		std::fill(blend_attachments.begin(), blend_attachments.end(), color_blend_attachment);

		for (size_t i = 0; i < vulkan_pipeline->m_ColorBlendSettings.size(); ++i)
		{
			blend_attachments[i].srcColorBlendFactor = TranslateBlendFactorToVulkan(vulkan_pipeline->m_ColorBlendSettings[i].m_SrcFactor);
			blend_attachments[i].dstColorBlendFactor = TranslateBlendFactorToVulkan(vulkan_pipeline->m_ColorBlendSettings[i].m_DstFactor);
			blend_attachments[i].colorBlendOp = TranslateBlendOpToVulkan(vulkan_pipeline->m_ColorBlendSettings[i].m_BlendOp);

			blend_attachments[i].srcAlphaBlendFactor = TranslateBlendFactorToVulkan(vulkan_pipeline->m_AlphaBlendSettings[i].m_SrcFactor);
			blend_attachments[i].dstAlphaBlendFactor = TranslateBlendFactorToVulkan(vulkan_pipeline->m_AlphaBlendSettings[i].m_DstFactor);
			blend_attachments[i].alphaBlendOp = TranslateBlendOpToVulkan(vulkan_pipeline->m_AlphaBlendSettings[i].m_BlendOp);

			blend_attachments[i].blendEnable = (vulkan_pipeline->m_ColorBlendSettings[i].IsNoBlendSetting() && vulkan_pipeline->m_AlphaBlendSettings[i].IsNoBlendSetting()) ? VK_FALSE : VK_TRUE;
			//switch (_config.m_ColorBlendModes[i])
			//{
			//case EPipelineBlendMode::E_BLEND_MODE_TRANSPARENT:
			//	blend_attachments[i].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			//	blend_attachments[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			//	blend_attachments[i].colorBlendOp = VK_BLEND_OP_ADD;
			//	blend_attachments[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			//	blend_attachments[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			//	blend_attachments[i].alphaBlendOp = VK_BLEND_OP_ADD;
			//	break;
			//case EPipelineBlendMode::E_BLEND_MODE_ADD:
			//	blend_attachments[i].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			//	blend_attachments[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
			//	blend_attachments[i].colorBlendOp = VK_BLEND_OP_ADD;
			//	blend_attachments[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			//	blend_attachments[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			//	blend_attachments[i].alphaBlendOp = VK_BLEND_OP_ADD;
			//	break;
			//case EPipelineBlendMode::E_BLEND_MODE_SRC_ZERO_DST_ONE_MINUS_SRC:
			//	blend_attachments[i].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			//	blend_attachments[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			//	blend_attachments[i].colorBlendOp = VK_BLEND_OP_ADD;
			//	blend_attachments[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			//	blend_attachments[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			//	blend_attachments[i].alphaBlendOp = VK_BLEND_OP_ADD;
			//	break;
			//case EPipelineBlendMode::E_BLEND_MODE_INVERSE_TRANSPARENT:
			//	blend_attachments[i].srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			//	blend_attachments[i].dstColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			//	blend_attachments[i].colorBlendOp = VK_BLEND_OP_ADD;
			//	blend_attachments[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			//	blend_attachments[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			//	blend_attachments[i].alphaBlendOp = VK_BLEND_OP_ADD;
			//	break;
			//default:
			//	break;
			//}
		}

		VkPipelineColorBlendStateCreateInfo color_blending = {};
		color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blending.logicOpEnable = VK_FALSE;
		color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
		color_blending.attachmentCount = blend_attachments_size;
		color_blending.pAttachments = blend_attachments.data();
		color_blending.blendConstants[0] = 0.0f; // Optional
		color_blending.blendConstants[1] = 0.0f; // Optional
		color_blending.blendConstants[2] = 0.0f; // Optional
		color_blending.blendConstants[3] = 0.0f; // Optional

		//dynamic states
		std::vector<VkDynamicState> dynamic_states = {
			VK_DYNAMIC_STATE_VIEWPORT,
			//VK_DYNAMIC_STATE_LINE_WIDTH,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamic_state_info = {};
		dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
		dynamic_state_info.pDynamicStates = dynamic_states.data();

		//pipeline layout
		VkPipelineLayout new_pipeline_layout;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		size_t set_layout_size = vulkan_pipeline->m_DescriptorSetLayouts.size();
		std::map<uint32_t, VkDescriptorSetLayout> layout_map;
		for (auto& layout : vulkan_pipeline->m_DescriptorSetLayouts)
		{
			VulkanDescriptorSetLayout* vulkan_layout = static_cast<VulkanDescriptorSetLayout*>(layout.second);
			layout_map.insert(std::make_pair(layout.first, vulkan_layout->m_DescriptorSetLayout));
		}
		//if (set_layout_size > 0)
		//{
		//	for (size_t i = 0; i < set_layout_size; ++i)
		//	{
		//		DescriptorSetPoolStruct& set_layout_struct = DescriptorSetPoolStructPool.getData(pipeline_struct.Config->m_DescriptorSetLayouts[i]);
		//		set_layouts[i] = set_layout_struct.SetLayout;
		//	}
		//	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(set_layout_size);
		//	pipelineLayoutInfo.pSetLayouts = set_layouts.data();
		//}
		if (m_VulkanSubpassInfos[subpass].m_InputLayout != VK_NULL_HANDLE)
		{
			layout_map.insert(std::make_pair(vulkan_pipeline->m_SubpassInputAttachmentBindPoint, m_VulkanSubpassInfos[subpass].m_InputLayout));

			////assert(set_layout_size < 2 && "Set 1 is already being used!");
			//++set_layout_size;
			//DescriptorSetPoolStruct& set_layout_struct = DescriptorSetPoolStructPool.getData(renderpass_struct.SubpassInputDescriptorSetLayouts[_stage]);
			////if (set_layout_size == 1)
			////{
			////	set_layouts.push_back(current_device.mEmptySetLayout);
			////	++set_layout_size;
			////}
			//set_layouts.push_back(set_layout_struct.SetLayout);
			//pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(set_layout_size);
			//pipelineLayoutInfo.pSetLayouts = set_layouts.data();
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
		//if (_config.m_PushConstantSize > 0)
		//{
		//	pipelineLayoutInfo.pushConstantRangeCount = 1;
		//	push_constant_range.offset = 0;
		//	push_constant_range.size = _config.m_PushConstantSize;
		//	push_constant_range.stageFlags = 0;
		//	for (size_t itr = 0; itr < static_cast<uint32_t>(EShaderType::SHADER_TYPE_MAX); ++itr)
		//	{
		//		if (_config.m_PushConstantShaderTypes & (1 << itr))
		//		{
		//			VkShaderStageFlagBits new_stage = FVulkan::ShaderStageTypeMap[itr];
		//			push_constant_range.stageFlags |= new_stage;
		//		}
		//	}
		//	pipelineLayoutInfo.pPushConstantRanges = &push_constant_range;
		//}

		VulkanDebug::CheckVKResult(vkCreatePipelineLayout(p_OwningDevice->m_LogicalDevice, &pipelineLayoutInfo, nullptr, &new_pipeline_layout), "Vulkan Pipeline Layout Creation Issue!");


		VkGraphicsPipelineCreateInfo pipeline_info = {};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount = static_cast<uint32_t>(shader_count);
		pipeline_info.pStages = shader_infos.data();

		VkPipelineTessellationStateCreateInfo tesselation_info = {};
		tesselation_info.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		tesselation_info.patchControlPoints = vulkan_pipeline->m_TesselationPatchSize;
		tesselation_info.flags = 0;

		pipeline_info.pVertexInputState = &vertex_input;
		pipeline_info.pInputAssemblyState = &input_assembly;
		pipeline_info.pTessellationState = tesselation_info.patchControlPoints > 0 ? &tesselation_info : nullptr;
		pipeline_info.pViewportState = &viewport_state;
		pipeline_info.pRasterizationState = &rasterizer;
		pipeline_info.pMultisampleState = &multisampling;
		pipeline_info.pDepthStencilState = &depth_stencil; // Optional
		pipeline_info.pColorBlendState = &color_blending;
		pipeline_info.pDynamicState = &dynamic_state_info; // Optional

		pipeline_info.layout = new_pipeline_layout;
		pipeline_info.renderPass = m_RenderPass;
		pipeline_info.subpass = subpass;
		pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipeline_info.flags = 0;
		pipeline_info.basePipelineIndex = -1; // Optional

		VkPipeline new_graphic_pipeline;
		VulkanDebug::CheckVKResult(vkCreateGraphicsPipelines(p_OwningDevice->m_LogicalDevice, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &new_graphic_pipeline), "Vulkan Pipeline Instantiation Issue!");
		m_VulkanSubpassInfos[subpass].m_PipelineInstances.insert(std::make_pair(pipeline, new_graphic_pipeline));
	}
	void VulkanRenderPass::Dispose()
	{
		if (m_RenderPass != VK_NULL_HANDLE)
		{
			vkDestroyRenderPass(p_OwningDevice->m_LogicalDevice, m_RenderPass, VULKAN_ALLOCATOR_POINTER);
			m_RenderPass = VK_NULL_HANDLE;

			for (auto& subpass : m_VulkanSubpassInfos)
			{
				if (subpass.m_InputLayout != VK_NULL_HANDLE)
				{
					vkDestroyDescriptorSetLayout(p_OwningDevice->m_LogicalDevice, subpass.m_InputLayout, VULKAN_ALLOCATOR_POINTER);
					subpass.m_InputLayout = VK_NULL_HANDLE;
				}
				for (auto& pipeline : subpass.m_PipelineInstances)
				{
					vkDestroyPipeline(p_OwningDevice->m_LogicalDevice, pipeline.second, VULKAN_ALLOCATOR_POINTER);
				}
				subpass.m_PipelineInstances.clear();
			}
		}
		p_OwningDevice->HandleDisposedRenderPass(this);
	}
}