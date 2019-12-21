#include <headers/VulkanRenderPass.h>
#include <headers/VulkanTranslator.h>
#include <headers/VulkanDebugLog.h>
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
		if (p_OwningDevice != nullptr)
		{
			m_VulkanSubpassInfos.clear();
			m_VulkanAttachmentInfos.clear();
			m_VulkanSubpassInfos.resize(m_Subpasses.size());
			std::vector<VkAttachmentDescription> vk_attachments;
			SubpassDependencyTracker dependency_tracker(m_Attachments.size());
			for (size_t attachment_id = 0; attachment_id < m_Attachments.size(); ++attachment_id)
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
				std::map<int32_t, size_t> output_set;
				std::map<int32_t, size_t> input_set;
				std::map<int32_t, size_t> depth_input_view_set;
				std::map<int32_t, size_t> stencil_input_view_set;
				std::map<int32_t, size_t> depth_stencil_view_set;
				for (size_t subpass_id = 0; subpass_id < m_Subpasses.size(); ++subpass_id)
				{
					std::map<int32_t, size_t> subpass_depth_input_set;
					std::map<int32_t, size_t> subpass_stencil_input_set;
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
							size_t new_attachment_index = m_VulkanAttachmentInfos.size() - 1;
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
							size_t new_attachment_index = m_VulkanAttachmentInfos.size() - 1;
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
							size_t new_attachment_index = m_VulkanAttachmentInfos.size() - 1;
							depth_input_view_set.insert(std::make_pair(attachment_ref, new_attachment_index));
							new_reference.attachment = new_attachment_index;
						}
						r_subpass_input_attachment_refs.push_back(new_reference);
						subpass_depth_input_set.insert(std::make_pair(attachment_ref, r_subpass_input_attachment_refs.size() - 1));
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
							size_t new_attachment_index = m_VulkanAttachmentInfos.size() - 1;
							stencil_input_view_set.insert(std::make_pair(attachment_ref, new_attachment_index));
							new_reference.attachment = new_attachment_index;
						}
						r_subpass_input_attachment_refs.push_back(new_reference);
						subpass_stencil_input_set.insert(std::make_pair(attachment_ref, r_subpass_input_attachment_refs.size()));
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
							size_t new_attachment_index = m_VulkanAttachmentInfos.size() - 1;
							depth_stencil_view_set.insert(std::make_pair(m_Subpasses[subpass_id].m_DepthStencilAttachment, new_attachment_index));
							new_reference.attachment = new_attachment_index;
						}
						{
							//check if depth input and stencil input of current depth / stencil attachment is used in current subpass
							auto depth_input_find = subpass_depth_input_set.find(m_Subpasses[subpass_id].m_DepthStencilAttachment);
							auto stencil_input_find = subpass_stencil_input_set.find(m_Subpasses[subpass_id].m_DepthStencilAttachment);
							bool depth_readonly = depth_input_find != subpass_depth_input_set.end();
							bool stencil_readonly = stencil_input_find != subpass_stencil_input_set.end();
							//if both depth stencil are treated as input attachment, then both channels should be readonly
							if (depth_readonly && stencil_readonly)
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
					current_subpass.inputAttachmentCount = r_subpass_input_attachment_refs.size();
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
						for (size_t binding_id = 0; binding_id < bindings.size(); ++binding_id)
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
						//switch (ref_attachment.m_ClearType)
						//{
						//case EAttachmentClearType::E_ATTACHMENT_NOT_CLEAR:
						//	current_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
						//	break;
						//case EAttachmentClearType::E_ATTACHMENT_CLEAR_ONES:
						//	current_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
						//	m_ClearValues[attachment_id] = { 1.0f, 1.0f, 1.0f, 1.0f };
						//	break;
						//case EAttachmentClearType::E_ATTACHMENT_CLEAR_ZEROS:
						//	current_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
						//	m_ClearValues[attachment_id] = { 0.0f, 0.0f, 0.0f, 0.0f };
						//	break;
						//}
						//current_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					}
					else
					{
						current_attachment.initialLayout = current_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
						//bool stencil_only = IsStencilOnlyFormat(ref_attachment.m_Format);
						//bool depth_only = IsDepthOnlyFormat(ref_attachment.m_Format);
						//switch (ref_attachment.m_ClearType)
						//{
						//case EAttachmentClearType::E_ATTACHMENT_NOT_CLEAR:
						//	current_attachment.stencilLoadOp = current_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
						//	break;
						//case EAttachmentClearType::E_ATTACHMENT_CLEAR_ONES:
						//	current_attachment.stencilLoadOp = current_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
						//	m_ClearValues[attachment_id] = { 0.0f, 0x0 };
						//	break;
						//case EAttachmentClearType::E_ATTACHMENT_CLEAR_ZEROS:
						//	current_attachment.stencilLoadOp = current_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
						//	m_ClearValues[attachment_id] = { 1.0f, 0x0 };
						//	break;
						//}
						//current_attachment.loadOp = stencil_only ? VK_ATTACHMENT_LOAD_OP_DONT_CARE : current_attachment.loadOp;
						//current_attachment.storeOp = stencil_only ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE;
						//current_attachment.stencilLoadOp = depth_only ? VK_ATTACHMENT_LOAD_OP_DONT_CARE : current_attachment.stencilLoadOp;
						//current_attachment.stencilStoreOp = depth_only ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE;
					}
					//TODO: Modifications
					current_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
				}
			}
			VkRenderPassCreateInfo create_info = {};
			create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			create_info.attachmentCount = m_VulkanAttachmentInfos.size();
			create_info.pAttachments = vk_attachments.data();
			create_info.subpassCount = static_cast<uint32_t>(m_Subpasses.size());
			create_info.pSubpasses = vk_subpasses.data();
			create_info.dependencyCount = static_cast<uint32_t>(vk_subpass_dependencies.size());
			create_info.pDependencies = vk_subpass_dependencies.size() > 0 ? vk_subpass_dependencies.data() : nullptr;

			VulkanDebug::CheckVKResult(vkCreateRenderPass(p_OwningDevice->m_LogicalDevice, &create_info, VULKAN_ALLOCATOR_POINTER, &m_RenderPass), "Vulkan Create RenderPass Issue!");
		}
	}
	void VulkanRenderPass::Dispose()
	{
		if (m_RenderPass != VK_NULL_HANDLE)
		{
			vkDestroyRenderPass(p_OwningDevice->m_LogicalDevice, m_RenderPass, VULKAN_ALLOCATOR_POINTER);
			m_RenderPass = VK_NULL_HANDLE;
			p_OwningDevice->HandleDisposedRenderPass(this);
		}
	}
}