#pragma once
#include <include/Pipeline.h>
#include <headers/VulkanGPUDevice.h>
#include <set>
#include <array>

namespace Crimson
{
	class VulkanDescriptorSet : public DescriptorSet
	{
	public:
		friend class VulkanGraphicsCommandBuffer;

		VulkanDescriptorSet();
		~VulkanDescriptorSet() {};

		virtual void WriteDescriptorSetBuffer(EBufferUniformType uniform_type, PGPUBuffer buffer, uint64_t buffer_offset, uint64_t write_size) override;
		virtual void EndWriteDescriptorSet() override;
		void SetVulkanDescriptorSet(VulkanGPUDevice* device, VkDescriptorSet set);
	private:
		VulkanGPUDevice*	p_OwningDevice;
		VkDescriptorSet		m_DescriptorSet;
		std::array<std::vector<VkDescriptorBufferInfo>, static_cast<size_t>(EBufferUniformType::E_BUFFER_UNIFORM_TYPE_MAX)>	m_BufferWriteCache;
		std::vector<VkDescriptorImageInfo>		m_ImageWriteCache;
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