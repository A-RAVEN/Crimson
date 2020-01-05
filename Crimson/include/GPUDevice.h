#pragma once
#include <string>
#include <vector>
#include <map>
#include "Generals.h"
#include "Pipeline.h"
#include "IWindow.h"

namespace Crimson
{
	enum class EAPIType : uint16_t
	{
		E_API_TYPE_VULKAN = 0,
		E_API_TYPE_D3D12,
		E_API_TYPE_MAX,
	};

	class GraphicsCommandBuffer
	{

	};
	using PGraphicsCommandBuffer = GraphicsCommandBuffer*;

	class IGPUDeviceThread
	{
	public:
		virtual PGraphicsCommandBuffer StartSubpassCommand(PRenderPassInstance renderpass_instance, uint32_t subpass_id) = 0;

	};
	using PGPUDeviceThread = IGPUDeviceThread*;

	class IGPUDevice : public IObjectManager
	{
	public:
		friend class GPUDeviceManager;

		virtual void InitDeviceChannel(uint32_t num_channel) = 0;
		virtual void RegisterWindow(IWindow& window) = 0;
		
		//Should Externally Symchronized
		virtual PGPUDeviceThread CreateThread() = 0;

		//Buffer Managing
		virtual PGPUBuffer CreateBuffer(uint64_t buffer_size, std::vector<EBufferUsage> const& usages, EMemoryType memory_type) = 0;
		//ImageManaging
		virtual PGPUImage CreateImage(EFormat format, uint32_t width, uint32_t height, uint32_t depth, std::vector<EImageUsage> const& usages, EMemoryType memory_type, uint32_t layer_num = 1, uint32_t mip_level_num = 1, uint32_t sample_num = 1) = 0;

		//RenderPass Managing
		virtual PRenderPass CreateRenderPass() = 0;

		//Descriptor Set Layout Managing
		virtual PDescriptorSetLayout CreateDescriptorSetLayout() = 0;

		//Pipeline Managing
		virtual PGraphicsPipeline CreateGraphicsPipeline() = 0;

		//Framebuffer Managing
		virtual PFramebuffer CreateFramebuffer() = 0;

		//RenderPass Instance Managing
		virtual PRenderPassInstance CreateRenderPassInstance(PRenderPass render_pass, PFramebuffer framebuffer) = 0;
	protected:
		std::string m_Name;

	};
	using PGPUDevice = IGPUDevice*;

	class GPUDeviceManager
	{
	public:
		static void Init();
		static void InitAPIContext(EAPIType type, bool enable_debug_system);
		static GPUDeviceManager* Get();
		static void Dispose();
		PGPUDevice CreateDevice(std::string const& name, uint32_t physics_device_id, EAPIType type, uint32_t prefered_graphics_queue_num, uint32_t prefered_compute_queue_num, uint32_t prefered_transfer_queue_num);
		PGPUDevice GetDevice(std::string const& name);
		PGPUDevice GetDevice(uint32_t id);
	private:
		GPUDeviceManager();
		~GPUDeviceManager();
		PGPUDevice CreateVulkanDevice(uint32_t physics_device_id, uint32_t prefered_graphics_queue_num, uint32_t prefered_compute_queue_num, uint32_t prefered_transfer_queue_num);

		static GPUDeviceManager*		p_Singleton;
		std::map<std::string, size_t>	m_DeviceMap;
		std::vector<PGPUDevice>			m_DeviceList;
	};
}