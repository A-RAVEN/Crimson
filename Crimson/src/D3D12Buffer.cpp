#include <headers/D3D12Buffer.h>
#include <headers/D3D12DebugLog.h>
namespace Crimson
{
	void D3D12BufferObject::SetD3D12Buffer(D3D12GPUDevice* p_device, uint64_t size, std::vector<EBufferUsage> const& usages, EMemoryType memory_type)
	{
        m_BufferSize = size;
        m_BufferUsages = usages;
        m_MemoryType = memory_type;

        p_OwningDevice = p_device;
        CHECK_DXRESULT(p_OwningDevice->m_Device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(size),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_Buffer)), "DX12 Buffer Creation Issue!");
	}
}