#pragma once
#include <headers/D3D12Header.h>
#include <include/Generals.h>
#include <headers/D3D12GPUDevice.h>
#include <headers/vk_mem_alloc.h>
#include <vector>

namespace Crimson
{
	class D3D12BufferObject : public IGPUBuffer
	{
	public:
		friend class VulkanGPUDevice;
		friend class VulkanGraphicsCommandBuffer;
		friend class VulkanExecutionCommandBuffer;
		friend class VulkanDescriptorSet;

		D3D12BufferObject();
		void SetD3D12Buffer(D3D12GPUDevice* p_device, uint64_t size, std::vector<EBufferUsage> const& usages, EMemoryType memory_type);
		//VkBufferView GetVulkanBufferView(std::string const& name);
		virtual void InitTexelBufferView(std::string const& name, EFormat format, uint64_t offset, uint64_t range) override {};
		virtual void Dispose() override;
		virtual uint8_t* GetMappedPointer() override;
		virtual void UnMapp() override;
		inline ComPtr<ID3D12Resource> GetD3D12BufferBuffer() const { return m_Buffer; }

	private:
		D3D12GPUDevice* p_OwningDevice;
		ComPtr<ID3D12Resource> m_Buffer;
		uint32_t			m_CurrentQueueFamily;
		uint8_t* p_Mapped;

		std::map<std::string, VkBufferView> m_BufferViews;
	};
}