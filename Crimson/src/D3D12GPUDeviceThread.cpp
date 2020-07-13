#include <headers/D3D12GPUDeviceThread.h>
#include <headers/D3D12DebugLog.h>
#include <headers/D3D12GPUDevice.h>
namespace Crimson
{
	D3D12GPUDeviceThread::D3D12GPUDeviceThread()
	{
	}
	D3D12GPUDeviceThread::~D3D12GPUDeviceThread()
	{
	}
	void D3D12GPUDeviceThread::Dispose()
	{
		m_GraphicsCommandPool.Reset();
		m_ComputeCommandPool.Reset();
		m_CopyCommandPool.Reset();
	}
	void D3D12GPUDeviceThread::InitGPUDeviceThread(D3D12GPUDevice* device)
	{
		p_OwningDevice = device;
	}
	void D3D12GPUDeviceThread::InitCommandAllocators()
	{
		CHECK_DXRESULT(p_OwningDevice->m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_GraphicsCommandPool)), "DX12 Direct Command Allocator Initialize Failed!");
		CHECK_DXRESULT(p_OwningDevice->m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_ComputeCommandPool)), "DX12 Compute Command Allocator Initialize Failed!");
		CHECK_DXRESULT(p_OwningDevice->m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CopyCommandPool)), "DX12 Copy Command Allocator Initialize Failed!");
	}
}