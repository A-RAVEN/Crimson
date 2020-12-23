#include <headers/D3D12Image.h>
#include <headers/D3D12DebugLog.h>
#include  <headers/D3D12Translator.h>
namespace Crimson
{
    D3D12ImageObject::D3D12ImageObject()
    {
    }
    void D3D12ImageObject::SetD3D12Image(D3D12GPUDevice* p_device, EFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mip_level_num, uint32_t layer_num, std::vector<EImageUsage> const& usages, EMemoryType memory_type)
	{
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
        for (auto usage : usages)
        {
            if (usage == EImageUsage::E_IMAGE_USAGE_COLOR_ATTACHMENT)
            {
                flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            }
            else if (usage == EImageUsage::E_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT)
            {
                flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
            }
        }
        p_OwningDevice = p_device;
        m_MemoryType = memory_type;
        m_Format = format;
        CD3DX12_RESOURCE_DESC textureDesc;
        if (height > 1)
        {
            if (depth > 1)
            {
                //tex 3d
                textureDesc = CD3DX12_RESOURCE_DESC::Tex3D(D3D12FormatType(format),
                    static_cast<UINT64>(width),
                    static_cast<UINT>(height),
                    static_cast<UINT16>(depth),
                    static_cast<UINT16>(mip_level_num),
                    flags
                );
            }
            else
            {
                //tex 2d
                textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(D3D12FormatType(format),
                    static_cast<UINT64>(width),
                    static_cast<UINT>(height),
                    static_cast<UINT16>(layer_num),
                    static_cast<UINT16>(mip_level_num),
                    1,
                    0,
                    flags
                );
            }
        }
        else
        {
            //tex 1d
            textureDesc = CD3DX12_RESOURCE_DESC::Tex1D(D3D12FormatType(format),
                static_cast<UINT64>(width),
                static_cast<UINT16>(layer_num),
                static_cast<UINT16>(mip_level_num),
                flags
            );
        }
        m_OverallState = D3D12_RESOURCE_STATE_COMMON;
        CHECK_DXRESULT(p_OwningDevice->m_Device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12MemoryTypeToHeapType(m_MemoryType)),
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            m_OverallState,
            nullptr,
            IID_PPV_ARGS(&m_Image)), "DX12 Image Creation Issue!");
	}
    void D3D12ImageObject::Dispose()
    {
        m_Image.Reset();
    }

    void D3D12ImageObject::TransitionOverallState(ComPtr<ID3D12GraphicsCommandList6> cmdList, D3D12_RESOURCE_STATES dstState)
    {
        if (m_OverallState == dstState) return;
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_Image.Get(), m_OverallState, dstState);
        cmdList->ResourceBarrier(1, &barrier);
        m_OverallState = dstState;
    }

}