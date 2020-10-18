#include <headers/D3D12GPUDeviceThread.h>
#include <headers/D3D12DebugLog.h>
#include <headers/D3D12GPUDevice.h>
#include <headers/D3D12RenderPassInstance.h>
#include <headers/D3D12Translator.h>
#include <headers/D3D12GraphicsCommandBuffer.h>
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
	PGraphicsCommandBuffer D3D12GPUDeviceThread::StartSubpassCommand(PRenderPassInstance renderpass_instance, uint32_t subpass_id)
	{
		D3D12RenderPassInstance* dxrenderpass_instance = static_cast<D3D12RenderPassInstance*>(renderpass_instance);
		auto& subpass = dxrenderpass_instance->m_SubpassInstances[subpass_id];
		ComPtr<ID3D12GraphicsCommandList4> commandList;
		auto find = m_SubpassCommandLists.find(dxrenderpass_instance);
		if (find == m_SubpassCommandLists.end())
		{
			m_SubpassCommandLists.insert(std::make_pair(dxrenderpass_instance, std::vector<ComPtr<ID3D12GraphicsCommandList4>>(dxrenderpass_instance->m_SubpassInstances.size(),
				ComPtr<ID3D12GraphicsCommandList4>{nullptr})));
			find = m_SubpassCommandLists.find(dxrenderpass_instance);
		}
		if (find->second[subpass_id] != nullptr)
		{
			find->second[subpass_id]->Reset(m_GraphicsCommandPool.Get(), NULL);
			commandList = find->second[subpass_id];
		}
		else
		{
			CHECK_DXRESULT(p_OwningDevice->m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_GraphicsCommandPool.Get(), nullptr, IID_PPV_ARGS(&commandList)), "D3D12 Create Subpass CommandList Issue!");
			find->second[subpass_id] = commandList;
		}
		commandList->BeginRenderPass(subpass.m_RenderTargetDescriptors.size(), subpass.m_RenderTargetDescriptors.data(), subpass.b_HasDepthStencil ? &subpass.m_DepthStencilDescriptor : NULL, D3D12_RENDER_PASS_FLAG_SUSPENDING_PASS | D3D12_RENDER_PASS_FLAG_RESUMING_PASS);
		D3D12GraphicsCommandBuffer* new_cmd_buffer = new D3D12GraphicsCommandBuffer();
		new_cmd_buffer->InitCommandBuffer(this, commandList, dxrenderpass_instance, subpass_id);
		return new_cmd_buffer;
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
		m_CopyCommandPool->Reset();
	}
}