#pragma once
#include <headers/D3D12Header.h>
#include <map>

namespace Crimson
{
	class D3D12DescriptorHeapWrapper
	{
	public:
		//using DescriptorRange = std::pair<size_t, size_t>;
		struct DescriptorRange
		{
		public:
			friend class D3D12DescriptorHeapWrapper;
			D3D12_CPU_DESCRIPTOR_HANDLE operator[](uint32_t index);
			D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t index);
			void Free();
		private:
			D3D12_CPU_DESCRIPTOR_HANDLE m_CPUStart;
			D3D12_GPU_DESCRIPTOR_HANDLE m_GPUStart;
			UINT64 m_Size;
			SIZE_T m_Stride;
			D3D12DescriptorHeapWrapper* m_Owner;
		};
		void Init(ComPtr<ID3D12Device2> p_device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT descriptor_num, D3D12_DESCRIPTOR_HEAP_FLAGS flags);
		DescriptorRange AllocDescriptor(size_t size);
		void FreeDescriptor(DescriptorRange const& range);
	private:
		ComPtr<ID3D12Device2> m_OwningDevice;
		ComPtr<ID3D12DescriptorHeap> m_Heap;
		std::map<UINT64, DescriptorRange> m_DescriptorChunks;
		SIZE_T m_Stride;
	};
}