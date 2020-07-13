#include <headers/D3D12GPUDevice.h>
#include <headers/D3D12DebugLog.h>
namespace Crimson
{
	void D3D12GPUDevice::RegisterWindow(IWindow& window)
	{
	}
	void D3D12GPUDevice::InitD3D12Device(ComPtr<IDXGIAdapter4>* p_adapter, uint32_t prefered_graphics_queue_num, uint32_t prefered_compute_queue_num, uint32_t prefered_transfer_queue_num)
	{
		p_Adapter = p_adapter;
		CHECK_DXRESULT(D3D12CreateDevice(p_Adapter->Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device)), "DX12 Device Creation Issue!");

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