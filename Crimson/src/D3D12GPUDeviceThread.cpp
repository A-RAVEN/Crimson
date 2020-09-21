#include <headers/D3D12GPUDeviceThread.h>
#include <headers/D3D12DebugLog.h>
#include <headers/D3D12GPUDevice.h>
#include <headers/D3D12Translator.h>
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
		InitCommandAllocators();
	}
	ComPtr<ID3D12GraphicsCommandList> D3D12GPUDeviceThread::AllocExecutionD3D12CommandList(EExecutionCommandType cmd_type, ComPtr<ID3D12CommandAllocator>& ownerAllocator)
	{
		ComPtr<ID3D12CommandAllocator> allocator= nullptr;
		switch (cmd_type)
		{
		case EExecutionCommandType::E_COMMAND_TYPE_GENERAL:
		case EExecutionCommandType::E_COMMAND_TYPE_GRAPHICS:
			allocator = m_GraphicsCommandPool;
			break;
		case EExecutionCommandType::E_COMMAND_TYPE_COMPUTE:
			allocator = m_ComputeCommandPool;
			break;
		case EExecutionCommandType::E_COMMAND_TYPE_COPY:
			allocator = m_CopyCommandPool;
			break;
		}
		ownerAllocator = allocator;

		ComPtr<ID3D12GraphicsCommandList> commandList;
		D3D12_COMMAND_LIST_TYPE commandType = D3D12ExecutionCommandTypeToCommandListType(cmd_type);
		CHECK_DXRESULT(p_OwningDevice->m_Device->CreateCommandList(0, commandType, allocator.Get(), nullptr, IID_PPV_ARGS(&commandList)), "D3D12 Create Execution Command Issue!");

		CHECK_DXRESULT(commandList->Close(), "D3D12 Close Execution Command Buffer Issue!");

		return commandList;
	}
	void D3D12GPUDeviceThread::RecycleExecutionD3D12CommandList(ComPtr<ID3D12GraphicsCommandList> cmd_buffer, EExecutionCommandType cmd_type)
	{
		cmd_buffer.Reset();
	}
	void D3D12GPUDeviceThread::InitCommandAllocators()
	{
		CHECK_DXRESULT(p_OwningDevice->m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_GraphicsCommandPool)), "DX12 Direct Command Allocator Initialize Failed!");
		CHECK_DXRESULT(p_OwningDevice->m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&m_ComputeCommandPool)), "DX12 Compute Command Allocator Initialize Failed!");
		CHECK_DXRESULT(p_OwningDevice->m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&m_CopyCommandPool)), "DX12 Copy Command Allocator Initialize Failed!");
		m_GraphicsCommandPool->Reset();
		m_ComputeCommandPool->Reset();
		m_GraphicsCommandPool->Reset();
	}
}