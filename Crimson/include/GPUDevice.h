#pragma once
#include <string>
#include <vector>
#include <map>
#include "Generals.h"
#include "Pipeline.h"
#include "RayTracer.h"
#include "RayTraceGeometry.h"
#include "IWindow.h"

namespace Crimson
{
	enum class EAPIType : uint16_t
	{
		E_API_TYPE_VULKAN = 0,
		E_API_TYPE_D3D12,
		E_API_TYPE_MAX,
	};

	enum class EExecutionCommandType : uint8_t
	{
		E_COMMAND_TYPE_GENERAL = 0,
		E_COMMAND_TYPE_GRAPHICS,
		E_COMMAND_TYPE_COMPUTE,
		E_COMMAND_TYPE_MAX
	};

	enum class EMemoryBarrierType : uint8_t
	{
		E_ACCEL_STRUCTURE_BUILD_READ_WRITE = 0,
		E_VERTEX_SHADER_READ_WRITE,
		E_HOST_READ_WRITE,
		E_MAX
	};

	enum class EMemoryOP : uint8_t
	{
		SHADER_READ = 0,
		SHADER_WRITE,
		ACCEL_STRUCTURE_READ,
		ACCEL_STRUCTURE_WRITE,
	};

	enum class EGPUStage : uint16_t
	{
		VERTEX_INPUT,
		VERTEX_SHADER,
		TESSCTR_SHADER,
		TESSEVL_SHADER,
		GEOMETRY_SHADER,
		FRAGMENT_SHADER,
		COMPUTE_SHADER,
		TASK_SHADER,
		MESH_SHADER,
		RAYTRACING,
	};

	class GraphicsCommandBuffer
	{
	public:
		virtual void EndCommandBuffer() = 0;
		virtual void BindSubpassDescriptorSets(std::vector<PDescriptorSet> const& descriptor_sets, uint32_t start_set = 0) = 0;
		virtual void ViewPort(float x, float y, float width, float height) = 0;
		virtual void Sissor(int _offsetx, int _offsety, uint32_t _extend_x, uint32_t _extend_y) = 0;
		virtual void BindSubpassPipeline(PGraphicsPipeline pipeline) = 0;
		virtual void BindVertexInputeBuffer(std::vector<PGPUBuffer> const& buffer_list, std::vector<uint64_t> const& buffer_offset_list) = 0;
		virtual void BindIndexBuffer(PGPUBuffer buffer, uint64_t buffer_offset, EIndexType index_type = EIndexType::E_INDEX_TYPE_32) = 0;
		virtual void DrawIndexed(uint32_t index_count, uint32_t instance_count,
			uint32_t first_index = 0, uint32_t first_vertex = 0, uint32_t first_instance_id = 0) = 0;
		virtual void Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance_id) = 0;
		virtual void DrawMeshShading(uint32_t task_count, uint32_t first_task_id) = 0;
	};
	using PGraphicsCommandBuffer = GraphicsCommandBuffer*;

	class ExecutionCommandBuffer
	{
	public:
		//Execute Render Pass, not thread safe for framebuffer images
		virtual void ExecuteRenderPassInstance(PRenderPassInstance renderpass_instance) = 0;
		virtual void CopyBufferToImage(PGPUBuffer buffer, PGPUImage image,
			uint64_t buffer_offset = 0, uint32_t mip_level = 0, uint32_t base_layer = 0, uint32_t layer_count = 1) = 0;
		virtual void CopyImageToImage(PGPUImage srd_image, PGPUImage dst_image) = 0;
		virtual void CopyToSwapchain_Dynamic(PGPUImage image, IWindow* p_window) = 0;
		virtual void BuildAccelerationStructure(PAccelerationStructure accel_struct, PGPUBuffer instance_buffer = nullptr, uint64_t instance_offset = 0, bool update = false) = 0;
		virtual void BindRayTracer(PRayTracer raytracer) = 0;
		virtual void BindRayTracingDescriptorSet(PDescriptorSet descriptor_set, uint32_t set_id) = 0;
		virtual void StartRayTracing(PGPUBuffer raygen_table, uint64_t raygen_offset, uint64_t miss_offset, uint64_t hit_offset, uint32_t width, uint32_t height) = 0;
		virtual void DeviceMemoryBarrier(EMemoryBarrierType barrier_type) = 0;
		virtual void StartCommand() = 0;
		virtual void EndCommand() = 0;
		virtual void LoadCache() = 0;
	protected:
		EExecutionCommandType m_CommandType;
	};
	using PExecutionCommandBuffer = ExecutionCommandBuffer*;

	class IGPUDeviceThread : public IObject
	{
	public:
		virtual PGraphicsCommandBuffer StartSubpassCommand(PRenderPassInstance renderpass_instance, uint32_t subpass_id) = 0;
		virtual PExecutionCommandBuffer CreateExecutionCommandBuffer(EExecutionCommandType cmd_type) = 0;
		virtual void BindExecutionCommandBufferToBatch(std::string const& batch_name, PExecutionCommandBuffer command_buffer, bool one_time = false) = 0;
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

		//Ray Tracer Managing
		virtual PRayTracer CreateRayTracer() = 0;

		//Framebuffer Managing
		virtual PFramebuffer CreateFramebuffer() = 0;

		//RenderPass Instance Managing
		virtual PRenderPassInstance CreateRenderPassInstance(PRenderPass render_pass, PFramebuffer framebuffer) = 0;

		//Ray Trace Geometry Managing 
		virtual PRayTraceGeometry CreateRayTraceGeometry() = 0;

		//Acceleration Structure Managing
		virtual PAccelerationStructure CreateAccelerationStructure() = 0;

		//Batch Managing
		virtual void CreateBatch(std::string const& batch_name, EExecutionCommandType command_type, uint32_t priority) = 0;
		virtual void DestroyBatch(std::string const& batch_name) = 0;
		virtual void ExecuteBatches(std::vector<std::string> const& batches) = 0;
		virtual void WaitBatches(std::vector<std::string> const& batches) = 0;

		virtual void WaitIdle() = 0;

		virtual void PresentWindow(IWindow& window) = 0;
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