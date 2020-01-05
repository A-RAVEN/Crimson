#pragma once
#include <headers/D3D12Header.h>
#include <include/GPUDevice.h>

namespace Crimson
{
	class D3D12GPUDevice : public IGPUDevice
	{
	public:
		friend class GPUDeviceManager;

		virtual void InitDeviceChannel(uint32_t num_channel) override {};
		virtual void RegisterWindow(IWindow& window) override;
	private:
		void InitD3D12Device();
	private:
		ComPtr<IDXGIAdapter4> m_Adapter;
		ComPtr<ID3D12Device2> m_Device;

		std::map<std::wstring, VulkanSurfaceContext> m_SurfaceContexts;

	};
}