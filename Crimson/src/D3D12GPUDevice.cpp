#include <headers/D3D12GPUDevice.h>
#include <headers/D3D12DebugLog.h>
#include <headers/D3D12Buffer.h>
#include <headers/D3D12Image.h>
#include <headers/D3D12ShaderModule.h>
namespace Crimson
{
	void D3D12GPUDevice::RegisterWindow(IWindow& window)
	{
		auto find = m_SurfaceContexts.find(window.GetName());
		if (find == m_SurfaceContexts.end())
		{
			D3D12SurfaceContext newContext{};
#ifdef _WIN32
			newContext.InitSurfaceContext(this, window);
#endif
			m_SurfaceContexts.insert(std::make_pair(window.GetName(), newContext));
		}
	}
	PGPUBuffer D3D12GPUDevice::CreateBuffer(uint64_t buffer_size, std::vector<EBufferUsage> const& usages, EMemoryType memory_type)
	{
		D3D12BufferObject* return_val = new D3D12BufferObject();
		return_val->SetD3D12Buffer(this, buffer_size, usages, memory_type);
		return return_val;
	}
	PGPUImage D3D12GPUDevice::CreateImage(EFormat format, uint32_t width, uint32_t height, uint32_t depth, std::vector<EImageUsage> const& usages, EMemoryType memory_type, uint32_t layer_num, uint32_t mip_level_num, uint32_t sample_num)
	{
		D3D12ImageObject* return_val = new D3D12ImageObject();
		return_val->SetD3D12Image(this, format, width, height, depth, mip_level_num, layer_num, usages, memory_type);
		return return_val;
	}
	PShaderModule D3D12GPUDevice::CreateShaderModule(void* data, size_t size, EShaderType shader_type)
	{
		ComPtr<ID3DBlob> blob;
		D3D12ShaderModule* return_val = new D3D12ShaderModule();
		return_val->Init(this, blob);
		return return_val;
	}
	void D3D12GPUDevice::InitD3D12Device(ComPtr<IDXGIAdapter4> p_adapter, uint32_t prefered_graphics_queue_num, uint32_t prefered_compute_queue_num, uint32_t prefered_transfer_queue_num)
	{
		p_Adapter = p_adapter;
		CHECK_DXRESULT(D3D12CreateDevice(p_Adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device)), "DX12 Device Creation Issue!");

		m_GraphicsQueues.resize(prefered_graphics_queue_num);
		m_ComputeQueues.resize(prefered_compute_queue_num);
		m_TransferQueues.resize(prefered_transfer_queue_num);

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		for (uint32_t i = 0; i < prefered_graphics_queue_num; ++i)
		{
			CHECK_DXRESULT(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_GraphicsQueues[i])), "DX12 Command Queue Creation Issue!");
		}

		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		for (uint32_t i = 0; i < prefered_compute_queue_num; ++i)
		{
			CHECK_DXRESULT(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_ComputeQueues[i])), "DX12 Command Queue Creation Issue!");
		}

		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		for (uint32_t i = 0; i < prefered_transfer_queue_num; ++i)
		{
			CHECK_DXRESULT(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_TransferQueues[i])), "DX12 Command Queue Creation Issue!");
		}

		InitDescriptorHeaps();
	}
	void D3D12GPUDevice::InitDescriptorHeaps()
	{
		D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
		HeapDesc.NumDescriptors = 100;
		HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		CHECK_DXRESULT(m_Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&m_DescriptorHeaps.m_BufferHeap)), "DX12 Create Buffer Heap Issue!");

		HeapDesc.NumDescriptors = 100;
		HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		CHECK_DXRESULT(m_Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&m_DescriptorHeaps.m_SamplerHeap)), "DX12 Create Sampler Heap Issue!");

		HeapDesc.NumDescriptors = 100;
		HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		CHECK_DXRESULT(m_Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&m_DescriptorHeaps.m_RtvHeap)), "DX12 Create Render Target Heap Issue!");

		HeapDesc.NumDescriptors = 100;
		HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		CHECK_DXRESULT(m_Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&m_DescriptorHeaps.m_DsvHeap)), "DX12 Create DSV Heap Issue!");
	}
}