#include <headers/D3D12Instance.h>
#include <headers/D3D12DebugLog.h>

namespace Crimson
{
	D3D12Instance* D3D12Instance::p_Singleton = nullptr;

	void D3D12Instance::Init(bool enable_debug_extension)
	{
		if (p_Singleton == nullptr)
		{
			p_Singleton = new D3D12Instance(enable_debug_extension);
		}
	}
	void D3D12Instance::Dispose()
	{
	}
	D3D12Instance* D3D12Instance::Get()
	{
		return p_Singleton;
	}
	D3D12Instance::D3D12Instance(bool enable_debug_extension)
	{
		UINT createFractoryFlags = 0;
		if (enable_debug_extension)
		{
			SetupDebugLayer();
			createFractoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
		CreateDXGIFactory2(createFractoryFlags, IID_PPV_ARGS(&dxgiFactory));

		EnumeratePhysicalDevices();
	}
	D3D12Instance::~D3D12Instance()
	{
	}
	void D3D12Instance::SetupDebugLayer()
	{
		CHECK_DXRESULT(D3D12GetDebugInterface(IID_PPV_ARGS(&m_DebugInterface)), "D3D12 Get Debug Interface Issue!");
		m_DebugInterface->EnableDebugLayer();
	}
	void D3D12Instance::DestroyDebugLayer()
	{
	}
	void D3D12Instance::EnumeratePhysicalDevices()
	{
		ComPtr<IDXGIAdapter1> dxgiAdapter1;
		ComPtr<IDXGIAdapter4> dxgiAdapter4;
		dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1));
		dxgiAdapter1.As(&dxgiAdapter4);
		{
			SIZE_T maxDedicatedVideoMemory = 0;
			for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
			{
				DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
				dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

				// Check to see if the adapter can create a D3D12 device without actually 
				// creating it. The adapter with the largest dedicated video memory
				// is favored.
				if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
					SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(),
						D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) &&
					dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
				{
					maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
					CHECK_DXRESULT(dxgiAdapter1.As(&dxgiAdapter4), "D3D12 Get Adapter4 Issue!");
					m_PhysicalDevices.push_back(dxgiAdapter4);
				}
			}
		}
	}
	void D3D12Instance::ClearPhysicalDevices()
	{
		m_PhysicalDevices.clear();
	}
}