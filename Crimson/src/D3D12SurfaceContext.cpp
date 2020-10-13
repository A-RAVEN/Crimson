#include <headers/D3D12SurfaceContext.h>
#include <headers/D3D12GPUDevice.h>
#include <headers/D3D12DebugLog.h>
#include <headers/D3D12Instance.h>

namespace Crimson
{
	void D3D12SurfaceContext::InitSurfaceContext(D3D12GPUDevice* device, IWindow& window)
	{
		p_OwningDevice = device;

		HWND win32Handle = window.GetWin32Handle();
		RECT windowRect{};
		GetWindowRect(win32Handle, &windowRect);


		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = windowRect.right - windowRect.left;
		swapChainDesc.Height = windowRect.bottom - windowRect.top;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.Stereo = FALSE;
		swapChainDesc.SampleDesc = { 1, 0 };
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		//Set Buffer Count as 3 for now
		swapChainDesc.BufferCount = 3;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		// It is recommended to always allow tearing if tearing support is available.
		swapChainDesc.Flags = CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		auto instance = D3D12Instance::Get();

		ComPtr<IDXGISwapChain1> swapChain1;

		CHECK_DXRESULT(instance->GetFactory4()->CreateSwapChainForHwnd(
			device->m_GraphicsQueues[0].Get(),
			win32Handle,
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain1), "D3D12 Create Swapchain For Window Issue!");

		// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
		// will be handled manually.
		CHECK_DXRESULT(instance->GetFactory4()->MakeWindowAssociation(win32Handle, DXGI_MWA_NO_ALT_ENTER), "D3D12 Toggle fullscreen feature Issue!");

		CHECK_DXRESULT(swapChain1.As(&swapChain4), "D3D12 Convert Swapchain 1 to 4 Issue!");

		UpdateRTViews();
	}

	void D3D12SurfaceContext::UpdateRTViews()
	{
		////increment size of render target view
		//auto rtvDescriptorSize = p_OwningDevice->m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		////start address of rtv heap
		//CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle();
		descriptorRange = p_OwningDevice->m_DescriptorHeaps.m_RtvHeap.AllocDescriptor(3);
		g_BackBuffers.resize(3);
		for (int i = 0; i < 3; ++i)
		{
			ComPtr<ID3D12Resource> backBuffer;
			CHECK_DXRESULT(swapChain4->GetBuffer(i, IID_PPV_ARGS(&backBuffer)), "D3D12 Swapchain Get Buffer Issue!");
			p_OwningDevice->m_Device->CreateRenderTargetView(backBuffer.Get(), nullptr, descriptorRange[i]);
			g_BackBuffers[i] = backBuffer;

			//offset address with size
			//rtvHandle.Offset(rtvDescriptorSize);
		}
	}

	bool D3D12SurfaceContext::CheckTearingSupport()
	{
		BOOL allowTearing = FALSE;

		// Rather than create the DXGI 1.5 factory interface directly, we create the
		// DXGI 1.4 interface and query for the 1.5 interface. This is to enable the 
		// graphics debugging tools which will not support the 1.5 factory interface 
		// until a future update.
		ComPtr<IDXGIFactory4> factory4;
		if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
		{
			ComPtr<IDXGIFactory5> factory5;
			if (SUCCEEDED(factory4.As(&factory5)))
			{
				if (FAILED(factory5->CheckFeatureSupport(
					DXGI_FEATURE_PRESENT_ALLOW_TEARING,
					&allowTearing, sizeof(allowTearing))))
				{
					allowTearing = FALSE;
				}
			}
		}
		return allowTearing == TRUE;
	}
}