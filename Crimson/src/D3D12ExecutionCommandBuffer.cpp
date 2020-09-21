#include <headers/D3D12ExecutionCommandBuffer.h>
#include <headers/D3D12GPUDevice.h>
#include <headers/D3D12GPUDeviceThread.h>
#include <headers/D3D12DebugLog.h>

namespace Crimson
{
	void D3D12ExecutionCommandBuffer::DeviceMemoryBarrier(EMemoryBarrierType barrier_type)
	{
		//CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::(
		//	backBuffer.Get(),
		//	D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)
		//m_CurrentCommandBuffer->ResourceBarrier()
	}
	void D3D12ExecutionCommandBuffer::StartCommand()
	{
		m_CurrentCommandBuffer->Reset(p_OwningAllocator.Get(), nullptr);
	}
	void D3D12ExecutionCommandBuffer::EndCommand()
	{
		CHECK_DXRESULT(m_CurrentCommandBuffer->Close(), "D3D12 Close Execution Command Buffer Issue!");
	}
	void D3D12ExecutionCommandBuffer::SetExecutionCommandBuffer(D3D12GPUDevice* p_device, D3D12GPUDeviceThread* p_thread, EExecutionCommandType command_type)
	{
		p_OwningDevice = p_device;
		p_OwningThread = p_thread;
		m_CommandType = command_type;
		m_CurrentCommandBuffer = p_OwningThread->AllocExecutionD3D12CommandList(m_CommandType, p_OwningAllocator);
	}
	D3D12ExecutionCommandBuffer::D3D12ExecutionCommandBuffer():
		p_OwningDevice(nullptr),
		p_OwningThread(nullptr),
		m_CommandType(EExecutionCommandType::E_COMMAND_TYPE_MAX),
		m_CurrentCommandBuffer(nullptr)
	{
	}
	void D3D12ExecutionCommandBuffer::CopyImageToImage(PGPUImage srd_image, PGPUImage dst_image)
	{

	}
}