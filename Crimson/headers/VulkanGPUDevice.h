#pragma once
#include "VulkanHeader.h"
#include <include/GPUDevice.h>
#include <include/IWindow.h>
#include <headers/vk_mem_alloc.h>
#include <headers/VulkanSurfaceContext.h>

namespace Crimson
{
	class VulkanBufferObject;
	class VulkanImageObject;
	class VulkanRenderPass;
	class VulkanGPUDevice : public IGPUDevice
	{
	public:
		//friends
		friend class GPUDeviceManager;
		friend class VulkanSurfaceContext;
		friend class VulkanRenderPass;
		
		virtual void InitDeviceChannel(uint32_t num_channel) override;
		virtual void RegisterWindow(IWindow& window) override;

		virtual PGPUDeviceThread CreateThread() override;

		//Buffer Managing
		virtual PGPUBuffer CreateBuffer(uint64_t buffer_size, std::vector<EBufferUsage> const& usages, EMemoryType memory_type) override;
		void HandleDisposedBuffer(VulkanBufferObject* p_buffer);

		//Image Managing
		virtual PGPUImage CreateImage(EFormat format, uint32_t width, uint32_t height, uint32_t depth, std::vector<EImageUsage> const& usages, EMemoryType memory_type, uint32_t layer_num, uint32_t mip_level_num, uint32_t sample_num) override;
		void HandleDisposedImage(VulkanImageObject* p_image);

		//RenderPass Managine
		virtual PRenderPass CreateRenderPass() override;
		void HandleDisposedRenderPass(VulkanRenderPass* p_renderpass);
	private:
		VulkanGPUDevice();
		~VulkanGPUDevice();
		void InitVulkanDevice(uint32_t prefered_device_index, uint32_t prefered_graphics_queue_num, uint32_t prefered_compute_queue_num, uint32_t prefered_transfer_queue_num);
		void RegisterVulkanSurface(VkSurfaceKHR surface);
		void InitMemoryAllocator();
	private:
		VkPhysicalDevice	m_PhysicalDevice;
		VkDevice			m_LogicalDevice;

		std::vector<VkQueueFamilyProperties>	m_QueueFamilies;
		std::vector<uint32_t>					m_QueueNumbers;
		uint32_t								m_GraphicsComputeGeneralFamily;
		uint32_t								m_GraphicsDedicateFamily;
		uint32_t								m_ComputeDedicateFamily;
		uint32_t								m_TransferDedicateFamily;
		uint32_t								m_SparseBindingFamily;

		//Surface Contexts
		std::map<std::wstring, VulkanSurfaceContext> m_SurfaceContexts;

		VmaAllocator m_MemoryAllocator;
	};
}