#pragma once
#include <include/GPUDevice.h>
#include <include/Generals.h>
#include <headers/D3D12Header.h>
#include <headers/D3D12GPUDevice.h>
#include <headers/D3D12DescriptorHeapWrapper.h>

namespace Crimson
{
	class D3D12Framebuffer : public Framebuffer
	{
	public:
		friend class D3D12RenderPassInstance;
		friend class D3D12ExecutionCommandBuffer;
		virtual void BuildFramebuffer() override;
		void Init(D3D12GPUDevice* owning_device)
		{
			p_OwningDevice = owning_device;
		}
	private:
		D3D12GPUDevice* p_OwningDevice;
		D3D12DescriptorHeapWrapper::DescriptorRange m_RTRanges;
		D3D12DescriptorHeapWrapper::DescriptorRange m_DSRanges;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_CPUHandles;
	};
}

