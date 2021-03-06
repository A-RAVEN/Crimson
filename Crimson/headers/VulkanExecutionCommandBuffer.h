#pragma once
#include <include/GPUDevice.h>
#include <include/DescriptorSets.h>
#include <headers/VulkanHeader.h>
#include <headers/VulkanGPUDevice.h>
#include <headers/VulkanGPUDeviceThread.h>
#include <headers/VulkanBatch.h>
#include <headers/VulkanImage.h>
#include <headers/VulkanAccelerationStructure.h>

namespace Crimson
{
	class VulkanExecutionCommandBuffer : public ExecutionCommandBuffer
	{
	public:
		friend class VulkanGPUDeviceThread;
		VulkanExecutionCommandBuffer();
		virtual void ExecuteRenderPassInstance(PRenderPassInstance renderpass_instance) override;
		virtual void CopyBufferToImage(PGPUBuffer buffer, PGPUImage image,
			uint64_t buffer_offset, uint32_t mip_level, uint32_t base_layer, uint32_t layer_count) override;
		virtual void CopyImageToImage(PGPUImage srd_image, PGPUImage dst_image) override;
		virtual void CopyToSwapchain_Dynamic(PGPUImage image, IWindow* p_window) override;
		virtual void BuildAccelerationStructure(PAccelerationStructure accel_struct, 
			PGPUBuffer instance_buffer, uint64_t instance_offset, bool update) override;
		//virtual void RayTrace(PGPUBuffer shader_table_buffer, uint32_t raygen_id, uint32_t miss_id, uint32_t )
		virtual void BindRayTracer(PRayTracer raytracer) override;
		virtual void BindRayTracingDescriptorSet(PDescriptorSet descriptor_set, uint32_t set_id) override;
		virtual void StartRayTracing(PGPUBuffer raygen_table, 
			uint64_t raygen_offset, uint64_t miss_offset, uint64_t hit_offset, 
			uint32_t width, uint32_t height) override;
		virtual void DeviceMemoryBarrier(EMemoryBarrierType barrier_type) override;
		virtual void StartCommand() override;
		virtual void EndCommand() override;
		virtual void LoadCache() override;
		void SetExecutionCommandBuffer(VulkanGPUDevice* p_device, VulkanGPUDeviceThread* p_thread, EExecutionCommandType command_type);
	private:
		void TransitionSwapchainImageToCopyDst(VkImage swapchain_image, VulkanSurfaceContext* surface_context, bool initialized);
		void TransitionSwapchainImageToPresent(VkImage swapchain_image, VulkanSurfaceContext* surface_context);
		VulkanGPUDevice* p_OwningDevice;
		VulkanGPUDeviceThread* p_OwningThread;
		VulkanBatch* p_AttachedBatch;
		EExecutionCommandType m_CommandType;
		VkCommandBuffer m_CurrentCommandBuffer;
		std::vector<VkSemaphore> m_AdditionialWaitingSemaphores;
		std::vector<VkPipelineStageFlags> m_AdditionalWaitingStages;

		//ray tracing
		VulkanRayTracer* p_CurrentBoundedRayTracer;

		std::map<VulkanImageObject*, VulkanImageLayoutCache> m_ImageLayoutCaches;
	};
}