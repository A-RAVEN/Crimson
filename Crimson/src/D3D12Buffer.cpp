#include <headers/D3D12Buffer.h>
#include <headers/D3D12DebugLog.h>
#include <headers/D3D12Translator.h>
namespace Crimson
{
    D3D12BufferObject::D3D12BufferObject() : 
        p_OwningDevice(nullptr),
        m_Buffer(nullptr),
        m_CurrentQueueFamily(std::numeric_limits<uint32_t>::max()),
        p_Mapped(nullptr)
    {
    }
    void D3D12BufferObject::SetD3D12Buffer(D3D12GPUDevice* p_device, uint64_t size, std::vector<EBufferUsage> const& usages, EMemoryType memory_type)
	{
        m_BufferSize = size;
        m_BufferUsages = usages;
        m_MemoryType = memory_type;

        D3D12_RESOURCE_STATES initialStates = D3D12_RESOURCE_STATE_COMMON;
        if (memory_type == EMemoryType::E_MEMORY_TYPE_HOST)
        {
            initialStates = D3D12_RESOURCE_STATE_GENERIC_READ;
        }
        else if (memory_type == EMemoryType::E_MEMORY_TYPE_DEVICE_TO_HOST)
        {
            initialStates = D3D12_RESOURCE_STATE_COPY_DEST;
        }

        p_OwningDevice = p_device;
        CHECK_DXRESULT(p_OwningDevice->m_Device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12MemoryTypeToHeapType(m_MemoryType)),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(size),
            initialStates,
            nullptr,
            IID_PPV_ARGS(&m_Buffer)), "DX12 Buffer Creation Issue!");
	}
	void D3D12BufferObject::Dispose()
	{
        m_Buffer.Reset();
	}
    uint8_t* D3D12BufferObject::GetMappedPointer()
    {
        if ((p_Mapped == nullptr) && (m_MemoryType == EMemoryType::E_MEMORY_TYPE_HOST || m_MemoryType == EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE || m_MemoryType == EMemoryType::E_MEMORY_TYPE_DEVICE_TO_HOST))
        {
            m_Buffer->Map(0, nullptr, reinterpret_cast<void**>(&p_Mapped));
        }
        return p_Mapped;
    }
    void D3D12BufferObject::UnMapp()
    {
        if (p_Mapped != nullptr)
        {
            m_Buffer->Unmap(0, nullptr);
        }
        p_Mapped = nullptr;
    }
}