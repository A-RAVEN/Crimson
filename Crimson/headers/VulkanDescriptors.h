#pragma once
#include <include/Pipeline.h>
#include <headers/VulkanGPUDevice.h>
#include <set>

namespace Crimson
{
	class VulkanDescriptorSet : public DescriptorSet
	{
	public:
		VulkanDescriptorSet();
		~VulkanDescriptorSet() {};
		void SetVulkanDescriptorSet(VulkanGPUDevice* device, VkDescriptorSet set);
	private:
		VulkanGPUDevice*	p_OwningDevice;
		VkDescriptorSet		m_DescriptorSet;
	};

	class VulkanDescriptorSetLayout : public DescriptorSetLayout
	{
	public:
		friend class VulkanRenderPass;

		VulkanDescriptorSetLayout(VulkanGPUDevice* device);
		~VulkanDescriptorSetLayout() {}
		virtual PDescriptorSet AllocDescriptorSet() override;
		virtual void BuildLayout() override;
		virtual void Dispose() override;
	private:
		VulkanGPUDevice*		p_OwningDevice;
		VkDescriptorSetLayout	m_DescriptorSetLayout;
	};
}