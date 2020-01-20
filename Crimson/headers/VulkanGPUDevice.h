#pragma once
#include "VulkanHeader.h"
#include <include/GPUDevice.h>
#include <include/IWindow.h>
#include <headers/vk_mem_alloc.h>
#include <headers/VulkanSurfaceContext.h>
#include <include/Pipeline.h>
#include <headers/HelperContainers.h>
#include <unordered_map>
#include <set>

namespace Crimson
{
	class VulkanBufferObject;
	class VulkanImageObject;
	class VulkanRenderPass;
	class VulkanDescriptorSetLayout;
	class VulkanGraphicsPipeline;
	class VulkanFramebuffer;
	class VulkanRenderPassInstance;
	class VulkanGPUDeviceThread;
	class VulkanBatch;
	class VulkanBufferObject;
	class VulkanGPUDevice : public IGPUDevice
	{
	public:
		//friends
		friend class GPUDeviceManager;
		friend class VulkanSurfaceContext;
		friend class VulkanRenderPass;
		friend class VulkanDescriptorSetLayout;
		friend class VulkanGraphicsPipeline;
		friend class VulkanImageObject;
		friend class VulkanBufferObject;
		friend class VulkanRenderPassInstance;
		friend class VulkanFramebuffer;
		friend class VulkanGPUDeviceThread;
		friend class VulkanDescriptorSet;
		friend class VulkanBatch;
		friend class VulkanExecutionCommandBuffer;
		
		virtual void InitDeviceChannel(uint32_t num_channel) override;
		virtual void RegisterWindow(IWindow& window) override;

		virtual PGPUDeviceThread CreateThread() override;
		virtual void HandleDisposedThread(VulkanGPUDeviceThread* p_thread);

		//Buffer Managing
		virtual PGPUBuffer CreateBuffer(uint64_t buffer_size, std::vector<EBufferUsage> const& usages, EMemoryType memory_type) override;
		void HandleDisposedBuffer(VulkanBufferObject* p_buffer);

		//Image Managing
		virtual PGPUImage CreateImage(EFormat format, uint32_t width, uint32_t height, uint32_t depth, std::vector<EImageUsage> const& usages, EMemoryType memory_type, uint32_t layer_num, uint32_t mip_level_num, uint32_t sample_num) override;
		void HandleDisposedImage(VulkanImageObject* p_image);

		//RenderPass Managine
		virtual PRenderPass CreateRenderPass() override;
		void HandleDisposedRenderPass(VulkanRenderPass* p_renderpass);

		//Descriptor Set Layout Managing
		virtual PDescriptorSetLayout CreateDescriptorSetLayout() override;
		void HandleDisposedDescriptorSetLayout(VulkanDescriptorSetLayout* p_set_layout);

		//Pipeline Managing
		virtual PGraphicsPipeline CreateGraphicsPipeline() override;
		void HandleDisposedGraphicsPipeline(VulkanGraphicsPipeline* p_pipeline);

		//Framebuffer Managing
		virtual PFramebuffer CreateFramebuffer() override;
		void HandleDisposedFramebuffer(VulkanFramebuffer* p_framebuffer);
		
		virtual PRenderPassInstance CreateRenderPassInstance(PRenderPass render_pass, PFramebuffer framebuffer) override;
		void HandleDisposedRenderPassInstance(VulkanRenderPassInstance* p_render_pass_instance);

		virtual void CreateBatch(std::string const& batch_name, EExecutionCommandType command_type, uint32_t priority) override;
		virtual void DestroyBatch(std::string const& batch_name) override;

		virtual void ExecuteBatches(std::vector<std::string> const& batches);
		virtual void PresentWindow(IWindow& window) override;

		std::vector<VkCommandBuffer> CollectSubpassCommandBuffers(uint32_t subpass_id, VulkanRenderPassInstance* p_instance);
		std::vector<VkCommandBuffer> CollectBatchCommandBuffers(uint32_t batch_id, std::vector<VkSemaphore> &waiting_semaphores, std::vector<VkPipelineStageFlags>& waiting_stages);
		uint32_t GetQueueFamilyIdByCommandType(EExecutionCommandType command_type);
	private:
		VulkanGPUDevice();
		~VulkanGPUDevice();
		void InitVulkanDevice(uint32_t prefered_device_index, uint32_t prefered_graphics_queue_num, uint32_t prefered_compute_queue_num, uint32_t prefered_transfer_queue_num);
		void InitMemoryAllocator();
		void InitDescriptorPool();
	private:
		VkPhysicalDevice	m_PhysicalDevice;
		VkDevice			m_LogicalDevice;

		VkDescriptorPool	m_DescriptorPool;

		std::vector<VkQueueFamilyProperties>	m_QueueFamilies;
		std::vector<uint32_t>					m_AllQueueFamilyIds;
		std::vector<uint32_t>					m_QueueNumbers;
		uint32_t								m_GraphicsComputeGeneralFamily;
		uint32_t								m_GraphicsDedicateFamily;
		uint32_t								m_ComputeDedicateFamily;
		uint32_t								m_TransferDedicateFamily;
		uint32_t								m_SparseBindingFamily;

		//Surface Contexts
		std::map<std::wstring, VulkanSurfaceContext> m_SurfaceContexts;

		VmaAllocator m_MemoryAllocator;

		IndexPool<uint32_t> m_RenderPassInstanceIdPool;

		std::set<VulkanGPUDeviceThread*> m_Threads;

		IndexPool<uint32_t> m_BatchIdPool;
		std::unordered_map<std::string, VulkanBatch*> m_Batches;
	};
}