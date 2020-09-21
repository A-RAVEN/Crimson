#pragma once
#include <headers/D3D12Header.h>
#include <include/GPUDevice.h>
#include <headers/D3D12SurfaceContext.h>

namespace Crimson
{
	class D3D12GPUDevice : public IGPUDevice
	{
	public:
		friend class GPUDeviceManager;
		friend class D3D12GPUDeviceThread;
		friend class D3D12BufferObject;
		friend class D3D12ImageObject;
		friend class D3D12SurfaceContext;
		friend class D3D12GraphicsPipeline;

		virtual void InitDeviceChannel(uint32_t num_channel) override {};
		virtual void RegisterWindow(IWindow& window) override;

		//Should Externally Symchronized
		virtual PGPUDeviceThread CreateThread() { return nullptr; };

		//Buffer Managing
		virtual PGPUBuffer CreateBuffer(uint64_t buffer_size, std::vector<EBufferUsage> const& usages, EMemoryType memory_type);
		//ImageManaging
		virtual PGPUImage CreateImage(EFormat format, uint32_t width, uint32_t height, uint32_t depth, std::vector<EImageUsage> const& usages, EMemoryType memory_type, uint32_t layer_num = 1, uint32_t mip_level_num = 1, uint32_t sample_num = 1);

		//RenderPass Managing
		virtual PRenderPass CreateRenderPass() { return nullptr; };

		//Descriptor Set Layout Managing
		virtual PDescriptorSetLayout CreateDescriptorSetLayout() { return nullptr; };

		//Shader Managing
		virtual PShaderModule CreateShaderModule(void* data, size_t size, EShaderType shader_type) override;

		//Pipeline Managing
		virtual PGraphicsPipeline CreateGraphicsPipeline() { return nullptr; };

		//Ray Tracer Managing
		virtual PRayTracer CreateRayTracer() { return nullptr; };

		//Framebuffer Managing
		virtual PFramebuffer CreateFramebuffer() { return nullptr; };

		//RenderPass Instance Managing
		virtual PRenderPassInstance CreateRenderPassInstance(PRenderPass render_pass, PFramebuffer framebuffer) { return nullptr; };

		//Ray Trace Geometry Managing 
		virtual PRayTraceGeometry CreateRayTraceGeometry() { return nullptr; };

		//Acceleration Structure Managing
		virtual PAccelerationStructure CreateAccelerationStructure() { return nullptr; };

		//Batch Managing
		virtual void CreateBatch(std::string const& batch_name, EExecutionCommandType command_type, uint32_t priority) {};
		virtual void DestroyBatch(std::string const& batch_name) {};
		virtual void ExecuteBatches(std::vector<std::string> const& batches) {};
		virtual void ExecuteBatches(std::vector<std::string> const& batches, EExecutionCommandType command_type, uint32_t queue_id) {};
		virtual void WaitBatches(std::vector<std::string> const& batches) {};

		virtual void WaitIdle() {};

		virtual void PresentWindow(IWindow& window) {};
	private:
		void InitD3D12Device(ComPtr<IDXGIAdapter4> p_adapter, uint32_t prefered_graphics_queue_num, uint32_t prefered_compute_queue_num, uint32_t prefered_transfer_queue_num);
		void InitDescriptorHeaps();
	private:
		ComPtr<IDXGIAdapter4> p_Adapter;
		ComPtr<ID3D12Device2> m_Device;

		std::map<std::wstring, D3D12SurfaceContext> m_SurfaceContexts;
		std::vector<ComPtr<ID3D12CommandQueue>> m_GraphicsQueues;
		std::vector<ComPtr<ID3D12CommandQueue>> m_ComputeQueues;
		std::vector<ComPtr<ID3D12CommandQueue>> m_TransferQueues;

		//
		struct DescriptorHeaps
		{
			ComPtr<ID3D12DescriptorHeap> m_BufferHeap;
			ComPtr<ID3D12DescriptorHeap> m_SamplerHeap;
			ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
			ComPtr<ID3D12DescriptorHeap> m_DsvHeap;
		} m_DescriptorHeaps;
	};
}