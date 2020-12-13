#pragma once
#include <headers/VulkanHeader.h>
#include <headers/VulkanGPUDevice.h>
#include <include/Generals.h>
#include <include/Pipeline.h>
#include <map>

namespace Crimson
{
	struct VulkanAttachmentInfo
	{
		uint32_t		m_ImageReference;
		VulkanAttachmentInfo() : 
			m_ImageReference(0)
		{}
	};

	struct VulkanSubpassInfo
	{
		std::map<GraphicsPipeline*, uint32_t> m_PipelineInstanceRefs;
		std::vector< std::pair<VkPipeline, VkPipelineLayout>> m_PipelineInstances;
		VkDescriptorSetLayout	m_InputLayout;
		VulkanSubpassInfo() :
			m_PipelineInstances(),
			m_InputLayout(VK_NULL_HANDLE)
		{}
	};

	class VulkanRenderPass : public RenderPass
	{
	public:
		friend class VulkanRenderPassInstance;
		friend class VulkanGraphicsCommandBuffer;
		friend class VulkanExecutionCommandBuffer;

		VulkanRenderPass(VulkanGPUDevice* owning_device);
		~VulkanRenderPass() {};
		virtual void BuildRenderPass() override;
		virtual GraphicsPipelineInstance InstanciatePipeline(GraphicsPipeline* pipeline, uint32_t subpass) override;
		virtual void Dispose() override;
	private:
		VulkanGPUDevice*					p_OwningDevice;
		VkRenderPass						m_RenderPass;

		std::vector<VulkanAttachmentInfo>	m_VulkanAttachmentInfos;
		std::vector<VkClearValue>			m_ClearValues;
		std::vector<VulkanSubpassInfo>		m_VulkanSubpassInfos;
	};
}