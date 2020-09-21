#include <headers/D3D12ShaderModule.h>

namespace Crimson
{
	void D3D12ShaderModule::Init(D3D12GPUDevice* p_device, ComPtr<ID3DBlob> &shader_blob)
	{
		m_Device = p_device;
		m_ShaderBlob = shader_blob;
	}
	void D3D12ShaderModule::Dispose()
	{
		m_ShaderBlob.Reset();
	}
}