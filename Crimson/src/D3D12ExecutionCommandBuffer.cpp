#include <headers/D3D12ExecutionCommandBuffer.h>
#include <headers/D3D12GPUDevice.h>
#include <headers/D3D12GPUDeviceThread.h>
#include <headers/D3D12RenderPassInstance.h>
#include <headers/D3D12DebugLog.h>
#include <headers/D3D12Image.h>
#include <headers/D3D12Translator.h>

namespace Crimson
{
	void D3D12ExecutionCommandBuffer::DeviceMemoryBarrier(EMemoryBarrierType barrier_type)
	{
	}
	void D3D12ExecutionCommandBuffer::StartCommand()
	{
		if (!p_OwningDevice->QueryQueueFenceSignalState(p_OwningAllocator.m_CmdQueueType, p_OwningAllocator.m_SubmitQueue, p_OwningAllocator.m_SubmitSignalValue))
		{
			p_OwningThread->ReturnCommandAllocator(p_OwningAllocator);
			p_OwningAllocator = p_OwningThread->AllocCommandAllocator(D3D12ExecutionCommandTypeToCommandListType(m_CommandType));
		}
		CHECK_DXRESULT(m_CurrentCommandBuffer->Reset(p_OwningAllocator.m_CommandAllocator.Get(), nullptr), "D3D12 CommandList Reset Issue!");
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

	DWORD WINAPI ThreadProc(LPVOID);

	void D3D12ExecutionCommandBuffer::Init(D3D12GPUDevice* device, D3D12GPUDeviceThread* thread, CommandAllocatorEntry const& allocator,
		EExecutionCommandType cmd_type, ComPtr<ID3D12GraphicsCommandList6> cmd_list)
	{
		p_OwningDevice = device;
		p_OwningThread = thread;
		p_OwningAllocator = allocator;
		device->m_Device->CreateFence(1, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&p_AllocatprFence));
		m_CommandType = cmd_type;
		m_CurrentCommandBuffer = cmd_list;
		p_WaitAllocatorEvent = CreateEvent(
			NULL,               // default security attributes
			TRUE,               // manual-reset event
			FALSE,              // initial state is nonsignaled
			TEXT("Wait Allocator"));
	}
	D3D12ExecutionCommandBuffer::D3D12ExecutionCommandBuffer():
		p_OwningDevice(nullptr),
		p_OwningThread(nullptr),
		m_CurrentCommandBuffer(nullptr)
	{
	}
	void D3D12ExecutionCommandBuffer::ExecuteRenderPassInstance(PRenderPassInstance renderpass_instance)
	{
		std::vector<ComPtr<ID3D12GraphicsCommandList4>> subpassCmdList;
		D3D12RenderPassInstance* dxinstance = static_cast<D3D12RenderPassInstance*>(renderpass_instance);
		for (auto pimg : dxinstance->p_DXFramebuffer->m_Images)
		{
			auto dximage = static_cast<D3D12ImageObject*>(pimg);
			dximage->TransitionOverallState(m_CurrentCommandBuffer, IsColorFormat(dximage->m_Format) ? D3D12_RESOURCE_STATE_RENDER_TARGET : D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}
		for (uint32_t i = 0; i < dxinstance->m_SubpassInstances.size(); ++i)
		{
			auto& sissor_setting = dxinstance->p_DXRenderPass->m_Subpasses[i].m_SissorsSetting;
			auto& viewport_setting = dxinstance->p_DXRenderPass->m_Subpasses[i].m_ViewportSetting;
			//ÉèÖÃviewport
			D3D12_VIEWPORT view_port{};
			view_port.TopLeftX = viewport_setting.x;
			view_port.TopLeftY = viewport_setting.y;
			view_port.Width = viewport_setting.width;
			view_port.Height = viewport_setting.height;
			view_port.MinDepth = 0.0f;
			view_port.MaxDepth = 1.0f;
			m_CurrentCommandBuffer->RSSetViewports(1, &view_port);
			//ÉèÖÃsissor
			D3D12_RECT rect{};
			rect.left = sissor_setting.x;
			rect.top = sissor_setting.y;
			rect.right = rect.left + sissor_setting.width;
			rect.bottom = rect.top + sissor_setting.height;
			m_CurrentCommandBuffer->RSSetScissorRects(1, &rect);
			subpassCmdList.clear();
			p_OwningDevice->CollectSubpassCommandLists(dxinstance, subpassCmdList, i);
			auto& subpass = dxinstance->m_SubpassInstances[i];
			m_CurrentCommandBuffer->BeginRenderPass(subpass.m_RenderTargetDescriptors.size(), subpass.m_RenderTargetDescriptors.data(), subpass.b_HasDepthStencil ? &subpass.m_DepthStencilDescriptor : NULL, D3D12_RENDER_PASS_FLAG_NONE);
			for (auto cmd : subpassCmdList)
			{
				m_CurrentCommandBuffer->ExecuteBundle(cmd.Get());
			}
			m_CurrentCommandBuffer->EndRenderPass();
		}
	}
	void D3D12ExecutionCommandBuffer::CopyImageToImage(PGPUImage srd_image, PGPUImage dst_image)
	{

	}
	void D3D12ExecutionCommandBuffer::CopyToSwapchain_Dynamic(PGPUImage image, IWindow* p_window)
	{
		auto find = p_OwningDevice->m_SurfaceContexts.find(p_window->GetName());
		if (find != p_OwningDevice->m_SurfaceContexts.end())
		{
			D3D12ImageObject* dxImage = static_cast<D3D12ImageObject*>(image);
			dxImage->TransitionOverallState(m_CurrentCommandBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);
			auto backBuffer = find->second.g_BackBuffers[find->second.m_CurrentFrameId];
			D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST);
			m_CurrentCommandBuffer->ResourceBarrier(1, &barrier);
			m_CurrentCommandBuffer->CopyResource(backBuffer.Get(), dxImage->m_Image.Get());
			barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);
			m_CurrentCommandBuffer->ResourceBarrier(1, &barrier);
			m_AdditionialWaitingFences.push_back(find->second.g_Fence);
		}
	}
}