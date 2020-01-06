#pragma once
#include <headers/D3D12Header.h>


namespace Crimson
{
	class D3D12GPUDevice;
	class D3D12SurfaceContext
	{
	public:
		friend class D3D12GPUDevice;
	private:
		D3D12GPUDevice* p_OwningDevice;

		ComPtr<IDXGISwapChain4> g_SwapChain;
		std::vector<ComPtr<ID3D12Resource>> g_BackBuffers;
	};
}