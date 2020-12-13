#include <headers/D3D12GPUDeviceThread.h>
#include <headers/D3D12DebugLog.h>
#include <headers/D3D12GPUDevice.h>
#include <headers/D3D12RenderPassInstance.h>
#include <headers/D3D12Translator.h>
#include <headers/D3D12GraphicsCommandBuffer.h>
#include <headers/D3D12ExecutionCommandBuffer.h>
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
		auto commandAllocator = AllocCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE);
		auto& subpass = dxrenderpass_instance->m_SubpassInstances[subpass_id];
		ComPtr<ID3D12GraphicsCommandList4> commandList;
		auto find = m_SubpassCommandLists.find(dxrenderpass_instance);
		if (find == m_SubpassCommandLists.end())
		{
			m_SubpassCommandLists.insert(std::make_pair(dxrenderpass_instance, std::vector<std::pair<ComPtr<ID3D12GraphicsCommandList4>, CommandAllocatorEntry>>(dxrenderpass_instance->m_SubpassInstances.size(),
				{ ComPtr<ID3D12GraphicsCommandList4>{nullptr}, CommandAllocatorEntry{} })));
			find = m_SubpassCommandLists.find(dxrenderpass_instance);
		}
		if (find->second[subpass_id].first != nullptr)
		{
			ReturnCommandAllocator(find->second[subpass_id].second);
			find->second[subpass_id].second = commandAllocator;
			find->second[subpass_id].first->Reset(commandAllocator.m_CommandAllocator.Get(), NULL);
			commandList = find->second[subpass_id].first;
		}
		else
		{
			CHECK_DXRESULT(p_OwningDevice->m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, commandAllocator.m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)), "D3D12 Create Subpass CommandList Issue!");
			find->second[subpass_id] = std::make_pair(commandList, commandAllocator);
		}
		D3D12GraphicsCommandBuffer* new_cmd_buffer = new D3D12GraphicsCommandBuffer();
		new_cmd_buffer->InitCommandBuffer(this, commandList, commandAllocator, dxrenderpass_instance, subpass_id);
		return new_cmd_buffer;
	}
	PExecutionCommandBuffer D3D12GPUDeviceThread::CreateExecutionCommandBuffer(EExecutionCommandType cmd_type)
	{
		D3D12ExecutionCommandBuffer* newCmdBuffer = new D3D12ExecutionCommandBuffer();
		CommandAllocatorEntry allocator;
		auto cmdList = AllocExecutionD3D12CommandList(cmd_type, allocator);
		newCmdBuffer->Init(p_OwningDevice, this, allocator, cmd_type, cmdList);
		return newCmdBuffer;
	}
	void D3D12GPUDeviceThread::BindExecutionCommandBufferToBatch(std::string const& batch_name, PExecutionCommandBuffer command_buffer, bool one_time)
	{
		auto find = p_OwningDevice->m_BatchIdMap.find(batch_name);
		CRIM_ASSERT_AND_RETURN_VOID(find != p_OwningDevice->m_BatchIdMap.end(), "Invalid Batch Name: " + batch_name);
		if (m_BatchDataList.size() <= find->second)
		{
			m_BatchDataList.resize(find->second + 1);
		}
		auto& batchData = m_BatchDataList[find->second];
		D3D12ExecutionCommandBuffer* dxcmdBuffer = static_cast<D3D12ExecutionCommandBuffer*>(command_buffer);
		switch (dxcmdBuffer->m_CommandType)
		{
		case EExecutionCommandType::E_COMMAND_TYPE_GENERAL:
		case EExecutionCommandType::E_COMMAND_TYPE_GRAPHICS:
			batchData.m_GraphicsExecutionBuffers.push_back(dxcmdBuffer);
			break;
		case EExecutionCommandType::E_COMMAND_TYPE_COMPUTE:
			batchData.m_ComputeExecutionBuffers.push_back(dxcmdBuffer);
			break;
		case EExecutionCommandType::E_COMMAND_TYPE_COPY:
			batchData.m_CopyExecutionBuffers.push_back(dxcmdBuffer);
			break;
		default:
			break;
		}
	}
	void D3D12GPUDeviceThread::InitGPUDeviceThread(D3D12GPUDevice* device)
	{
		p_OwningDevice = device;
		InitCommandAllocators();
	}
	ComPtr<ID3D12GraphicsCommandList4> D3D12GPUDeviceThread::GetSubpassCommandList(PRenderPassInstance instance, uint32_t subpassId)
	{
		auto find = m_SubpassCommandLists.find(instance);
		if (find != m_SubpassCommandLists.end())
		{
			return find->second[subpassId].first;
		}
		return ComPtr<ID3D12GraphicsCommandList4>(nullptr);
	}
	ComPtr<ID3D12GraphicsCommandList6> D3D12GPUDeviceThread::AllocExecutionD3D12CommandList(EExecutionCommandType cmd_type, CommandAllocatorEntry& ownerAllocator)
	{
		D3D12_COMMAND_LIST_TYPE commandType = D3D12ExecutionCommandTypeToCommandListType(cmd_type);
		ownerAllocator = AllocCommandAllocator(commandType);
		ComPtr<ID3D12GraphicsCommandList6> commandList;
		CHECK_DXRESULT(p_OwningDevice->m_Device->CreateCommandList(0, commandType, ownerAllocator.m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)), "D3D12 Create Execution Command Issue!");

		CHECK_DXRESULT(commandList->Close(), "D3D12 Close Execution Command Buffer Issue!");

		return commandList;
	}
	void D3D12GPUDeviceThread::RecycleExecutionD3D12CommandList(ComPtr<ID3D12GraphicsCommandList6> cmd_buffer, EExecutionCommandType cmd_type)
	{
		cmd_buffer.Reset();
	}
	CommandAllocatorEntry D3D12GPUDeviceThread::AllocCommandAllocator(D3D12_COMMAND_LIST_TYPE type)
	{
		if (!m_CommandAllocatorQueues[type].empty())
		{
			auto front = m_CommandAllocatorQueues[type].front();
			if (p_OwningDevice->QueryQueueFenceSignalState(front.m_CmdQueueType, front.m_SubmitQueue, front.m_SubmitSignalValue))
			{
				auto val = front;
				val.m_SubmitQueue = -1;
				val.m_SubmitSignalValue = -1;
				m_CommandAllocatorQueues->pop();
				val.m_CommandAllocator->Reset();
				return val;
			}
		}
		CommandAllocatorEntry return_val = { nullptr, type, -1, -1 };
		CHECK_DXRESULT(p_OwningDevice->m_Device->CreateCommandAllocator(type, IID_PPV_ARGS(&return_val.m_CommandAllocator)), "DX12 Create New Command Allocator Issue!");
		return return_val;
	}
	void D3D12GPUDeviceThread::ReturnCommandAllocator(CommandAllocatorEntry& return_entry)
	{
		m_CommandAllocatorQueues[return_entry.m_CmdQueueType].push(return_entry);
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
	void D3D12GPUDeviceThread::ThreadBatchData::CollectCmdLists(EExecutionCommandType cmdType, std::vector<ID3D12CommandList *>& list, std::vector<ID3D12Fence*>& allocator_fences, uint32_t queue_id, uint64_t signal_val)
	{
		std::vector<D3D12ExecutionCommandBuffer*> const* targetList = nullptr;
		switch (cmdType)
		{
		case EExecutionCommandType::E_COMMAND_TYPE_GENERAL:
		case EExecutionCommandType::E_COMMAND_TYPE_GRAPHICS:
			targetList = &m_GraphicsExecutionBuffers;
			break;
		case EExecutionCommandType::E_COMMAND_TYPE_COMPUTE:
			targetList = &m_ComputeExecutionBuffers;
			break;
		case EExecutionCommandType::E_COMMAND_TYPE_COPY:
			targetList = &m_CopyExecutionBuffers;
			break;
		default:
			break;
		}
		if (targetList == nullptr) { return; }
		for (auto p_buffer : *targetList)
		{
			list.push_back(p_buffer->m_CurrentCommandBuffer.Get());
			p_buffer->p_OwningAllocator.m_CmdQueueType = D3D12ExecutionCommandTypeToCommandListType(cmdType);
			p_buffer->p_OwningAllocator.m_SubmitQueue = queue_id;
			p_buffer->p_OwningAllocator.m_SubmitSignalValue = signal_val;
			//allocator_fences.push_back(p_buffer->p_AllocatprFence.Get());
		}
	}
}