#include <headers/D3D12DescriptorHeapWrapper.h>
#include <headers/D3D12DebugLog.h>
#include <headers/GeneralDebug.h>

namespace Crimson
{
	void D3D12DescriptorHeapWrapper::Init(ComPtr<ID3D12Device2> p_device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT descriptor_num, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
	{
		m_OwningDevice = p_device;
		D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
		HeapDesc.NumDescriptors = descriptor_num;
		HeapDesc.Type = type;
		HeapDesc.Flags = flags;
		CHECK_DXRESULT(m_OwningDevice->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&m_Heap)), "DX12 Heap Issue!");
		DescriptorRange newRange{};
		newRange.m_CPUStart = m_Heap->GetCPUDescriptorHandleForHeapStart();
		newRange.m_GPUStart = m_Heap->GetGPUDescriptorHandleForHeapStart();
		newRange.m_Size = descriptor_num;
		m_DescriptorChunks.insert(std::make_pair(newRange.m_CPUStart.ptr, newRange));
	}
	D3D12DescriptorHeapWrapper::DescriptorRange D3D12DescriptorHeapWrapper::AllocDescriptor(size_t size)
	{
		CRIM_ASSERT_AND_RETURN(size > 0, "D3D12 Cannot alloc descriptors with size 0!", DescriptorRange{});
		UINT64 minSize = std::numeric_limits<UINT64>::max();
		std::map<UINT64, DescriptorRange>::iterator closestChunk = m_DescriptorChunks.end();
		auto currentChunk = m_DescriptorChunks.begin();
		auto chunkEnd = m_DescriptorChunks.end();
		for (currentChunk; currentChunk != chunkEnd; ++currentChunk)
		{
			if (currentChunk->second.m_Size >= size && currentChunk->second.m_Size < minSize)
			{
				minSize = currentChunk->second.m_Size;
				closestChunk = currentChunk;
			}
		}
		if (closestChunk != m_DescriptorChunks.end())
		{
			DescriptorRange chosenChunk = closestChunk->second;
			m_DescriptorChunks.erase(closestChunk);
			if (chosenChunk.m_Size > size)
			{
				//split chosen chunk
				DescriptorRange splittedChunk{};
				splittedChunk.m_CPUStart.ptr = chosenChunk.m_CPUStart.ptr + size;
				splittedChunk.m_GPUStart.ptr = chosenChunk.m_GPUStart.ptr + size;
				splittedChunk.m_Size = chosenChunk.m_Size - size;
				m_DescriptorChunks.insert(std::make_pair(splittedChunk.m_CPUStart.ptr, splittedChunk));
				chosenChunk.m_Size = size;
			}
			chosenChunk.m_Owner = this;
			return chosenChunk;
		}
		LOG_ERR("D3D12 Descriptor Chunk Allocation Failed!");
		return DescriptorRange{};
	}
	void D3D12DescriptorHeapWrapper::FreeDescriptor(DescriptorRange const& range)
	{
		CRIM_ASSERT_AND_RETURN_VOID(m_DescriptorChunks.find(range.m_CPUStart.ptr) == m_DescriptorChunks.end(), "D3D12 Cannot free descriptors multiple times!");
		m_DescriptorChunks.insert(std::make_pair(range.m_CPUStart.ptr, range));
		auto find = m_DescriptorChunks.find(range.m_CPUStart.ptr);
		auto previous = find;
		auto next = find;
		++next;
		if (previous != m_DescriptorChunks.begin())
		{
			--previous;
		}
		//try combine find and previous
		if (previous != find)
		{
			if (previous->second.m_CPUStart.ptr + previous->second.m_Size == find->second.m_CPUStart.ptr)
			{
				previous->second.m_Size += find->second.m_Size;
				m_DescriptorChunks.erase(find);
				find = previous;
				next = find;
				++next;
			}
		}
		if (next != m_DescriptorChunks.end())
		{
			if (find->second.m_CPUStart.ptr + find->second.m_Size == next->second.m_CPUStart.ptr)
			{
				find->second.m_Size += next->second.m_Size;
				m_DescriptorChunks.erase(next);
			}
		}
	}
	D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeapWrapper::DescriptorRange::operator[](uint32_t index)
	{
		CRIM_ASSERT_AND_RETURN(index < m_Size, "D3D12 Descriptor Indexing Out of Bound!", D3D12_CPU_DESCRIPTOR_HANDLE{});
		D3D12_CPU_DESCRIPTOR_HANDLE returnVal = m_CPUStart;
		returnVal.ptr += index;
		return returnVal;
	}
	D3D12_GPU_DESCRIPTOR_HANDLE D3D12DescriptorHeapWrapper::DescriptorRange::GetGPUHandle(uint32_t index)
	{
		CRIM_ASSERT_AND_RETURN(index < m_Size, "D3D12 Descriptor Indexing Out of Bound!", D3D12_GPU_DESCRIPTOR_HANDLE{});
		D3D12_GPU_DESCRIPTOR_HANDLE returnVal = m_GPUStart;
		returnVal.ptr += index;
		return returnVal;
	}
	void D3D12DescriptorHeapWrapper::DescriptorRange::Free()
	{
		m_Owner->FreeDescriptor(*this);
	}
}