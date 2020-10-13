#pragma once
#include <headers/D3D12Header.h>
#include <include/IWindow.h>

namespace Crimson
{
	class D3D12GPUDevice;
	class D3D12SurfaceContext
	{
	public:
		friend class D3D12GPUDevice;
	private:
		void InitSurfaceContext(D3D12GPUDevice* device, IWindow& window);
		void UpdateRTViews();
		bool CheckTearingSupport();
		D3D12GPUDevice* p_OwningDevice;

		ComPtr<IDXGISwapChain4> g_SwapChain;
		std::vector<ComPtr<ID3D12Resource>> g_BackBuffers;

		D3D12DescriptorHeapWrapper::DescriptorRange descriptorRange;

		ComPtr<IDXGISwapChain4> swapChain4;
	};
}