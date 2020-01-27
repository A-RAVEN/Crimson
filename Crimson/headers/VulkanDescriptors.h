#pragma once
#include <include/Pipeline.h>
#include <headers/VulkanGPUDevice.h>
#include <set>
#include <array>

namespace Crimson
{
	class VulkanDescriptorSetLayout;
	class VulkanDescriptorSetBufferWriteInfo
	{
	public:
		std::vector<VkDescriptorBufferInfo> buffer_infos;
		uint32_t m_ArrayElement;
		uint32_t m_Binding;
	};
	class VulkanDescriptorSet : public DescriptorSet
	{
	public:
		friend class VulkanGraphicsCommandBuffer;

		VulkanDescriptorSet();
		~VulkanDescriptorSet() {};

		//virtual void WriteDescriptorSetBuffer(uint32_t binding_point, EBufferUniformType uniform_type, 
		//	PGPUBuffer buffer, uint64_t buffer_offset, uint64_t write_size) override;
		virtual void WriteDescriptorSetBuffers(uint32_t binding_point,
			std::vector<PGPUBuffer> const& buffers, std::vector<BufferRange> const& buffer_ranges, uint32_t start_array_id) override;
		virtual void WriteDescriptorSetImage(uint32_t binding_point,
			PGPUImage image, EFilterMode filter_mode, EAddrMode addr_mode, EViewAsType view_as = EViewAsType::E_VIEW_AS_TYPE_MAX, uint32_t array_id = 0) override;
		virtual void EndWriteDescriptorSet() override;
		void SetVulkanDescriptorSet(VulkanGPUDevice* device, VkDescriptorSet set, VulkanDescriptorSetLayout* p_layout);
	private:
		VulkanGPUDevice*	p_OwningDevice;
		VulkanDescriptorSetLayout* p_OwningSetLayout;
		VkDescriptorSet		m_DescriptorSet;
		std::vector<std::vector<VkDescriptorBufferInfo>> m_BufferWriteInfoCache;
		std::vector<VkDescriptorImageInfo> m_ImageWriteInfo;
		std::vector<VkWriteDescriptorSet> m_WriteCache;

	};

	class VulkanDescriptorSetLayout : public DescriptorSetLayout
	{
	public:
		friend class VulkanRenderPass;
		friend class VulkanRayTracer;

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