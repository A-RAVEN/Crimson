#include <headers/D3D12GPUDevice.h>

namespace Crimson
{
	void D3D12GPUDevice::RegisterWindow(IWindow& window)
	{
	}
	void D3D12GPUDevice::InitD3D12Device()
	{
		D3D12CreateDevice(m_Adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device));
	}
}