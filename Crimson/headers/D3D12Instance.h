#pragma once
#include <headers/D3D12Header.h>

namespace Crimson
{

	class D3D12Instance
	{
	public:
		static void Init(bool enable_debug_extension);
		static void Dispose();
		static D3D12Instance* Get();
	private:
		D3D12Instance(bool enable_debug_extension);
		~D3D12Instance();
		//Debug Callbacks
		void SetupDebugLayer();
		void DestroyDebugLayer();

		//Physical Devices
		void EnumeratePhysicalDevices();
		void ClearPhysicalDevices();

		static D3D12Instance* p_Singleton;

		ComPtr<ID3D12Debug> m_DebugInterface;
		ComPtr<IDXGIFactory4> dxgiFactory;

	};
}