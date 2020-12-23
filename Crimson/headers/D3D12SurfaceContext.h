#pragma once
#include <headers/D3D12Header.h>
#include <include/IWindow.h>
#include <headers/D3D12DescriptorHeapWrapper.h>

namespace Crimson
{
	class D3D12GPUDevice;
	class D3D12SurfaceContext
	{
	public:
		friend class D3D12GPUDevice;
		friend class D3D12ExecutionCommandBuffer;
	private:
		void InitSurfaceContext(D3D12GPUDevice* device, IWindow& window);
		void UpdateRTViews();
		bool CheckTearingSupport();
		D3D12GPUDevice* p_OwningDevice;

		//ComPtr<IDXGISwapChain4> g_SwapChain;
		std::vector<ComPtr<ID3D12Resource>> g_BackBuffers;

		D3D12DescriptorHeapWrapper::DescriptorRange descriptorRange;

		ComPtr<IDXGISwapChain4> swapChain4;
		uint32_t m_CurrentFrameId;

		bool g_VSync = true;
		bool g_TearingSupported = false;

		ComPtr<ID3D12Fence> g_Fence;
		uint64_t g_FenceValue = 1;
		uint64_t g_FrameFenceValues[3] = {};
		HANDLE g_FenceEvent;
	};
}