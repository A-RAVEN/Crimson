#include <headers/D3D12Framebuffer.h>
#include <headers/D3D12Image.h>
#include <headers/D3D12Translator.h>

namespace Crimson
{
	void D3D12Framebuffer::BuildFramebuffer()
	{
		std::vector<D3D12ImageObject*> dsFrames;
		std::vector<D3D12ImageObject*> rtFrames;
		std::vector<uint32_t> dsIds;
		std::vector<uint32_t> rtIds;
		m_CPUHandles.resize(m_Images.size());
		for (uint32_t i = 0; i < m_Images.size(); ++i)
		{
			auto image = m_Images[i];
			D3D12ImageObject* dximage = static_cast<D3D12ImageObject*>(image);
			if (IsDepthStencilFormat(dximage->GetFormat()))
			{
				dsFrames.push_back(dximage);
				dsIds.push_back(i);
			}
			else
			{
				rtFrames.push_back(dximage);
				rtIds.push_back(i);
			}
		}
		if (rtFrames.size() > 0)
		{
			m_RTRanges = p_OwningDevice->m_DescriptorHeaps.m_Heaps[D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV].AllocDescriptor(rtFrames.size());
		}
		if (dsFrames.size() > 0)
		{
			m_DSRanges = p_OwningDevice->m_DescriptorHeaps.m_Heaps[D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV].AllocDescriptor(dsFrames.size());
		}
		for (uint32_t i = 0; i < rtFrames.size(); ++i)
		{
			D3D12ImageObject* dximage = rtFrames[i];
			D3D12_RENDER_TARGET_VIEW_DESC desc{};
			desc.Texture2D.MipSlice = 0;
			desc.Texture2D.PlaneSlice = 0;
			desc.Format = D3D12FormatType(dximage->GetFormat());
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			p_OwningDevice->m_Device->CreateRenderTargetView(dximage->m_Image.Get(), &desc, m_RTRanges[i]);
			p_OwningDevice->Diagnose();
			m_CPUHandles[rtIds[i]] = m_RTRanges[i];
		}
		for (uint32_t i = 0; i < dsFrames.size(); ++i)
		{
			D3D12ImageObject* dximage = dsFrames[i];
			p_OwningDevice->m_Device->CreateDepthStencilView(dximage->m_Image.Get(), nullptr, m_DSRanges[i]);
			m_CPUHandles[dsIds[i]] = m_DSRanges[i];
		}
	}
}