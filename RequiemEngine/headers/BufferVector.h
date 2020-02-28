#pragma once
#include <Generals.h>
#include <GPUDevice.h>
#include <vector>

using namespace Crimson;
template<typename T, size_t segment_size>
class BufferQueue
{
public:
	BufferQueue() :
		m_CurrentLastId(0),
		m_Capability(0),
		p_Device(nullptr)
	{}
	void Init(PGPUDevice device, std::vector<EBufferUsage> const& usages, EMemoryType memory_type)
	{
		p_Device = device;
		m_Usages = usages;
		m_MemoryType = memory_type;
	}
	bool Inited() const { return p_Device != nullptr; }
	void PushBack(T const& val);
	void PopBack();
	size_t Size() const;
	void Clear();
	T& operator[](size_t id);
	T const& operator[](size_t id) const;
	PGPUBuffer GetBufferSegment(size_t segment_id) { return m_Buffers[segment_id].second; }
	size_t GetSegmentActiveUsage(size_t segment_id) const 
	{
		return (std::min)(m_CurrentLastId - segment_id * segment_size, segment_size);
	}
	size_t GetActiveSegmentNum() const { return m_CurrentLastId / segment_size + (m_CurrentLastId % segment_size > 0 ? 1 : 0); }
private:
	std::vector<EBufferUsage> m_Usages;
	EMemoryType m_MemoryType;
	PGPUDevice p_Device;
	size_t m_CurrentLastId;
	size_t m_Capability;
	std::vector<std::pair<T*, PGPUBuffer>> m_Buffers;
};

template<typename T, size_t segment_size>
inline void BufferQueue<T, segment_size>::PushBack(T const& val)
{
	if (m_CurrentLastId >= m_Capability)
	{
		PGPUBuffer new_buffer = p_Device->CreateBuffer(sizeof(T) * segment_size, m_Usages, m_MemoryType);
		T* t = new (new_buffer->GetMappedPointer()) T[segment_size];
		m_Buffers.push_back(std::make_pair(t, new_buffer));
		m_Capability += segment_size;
	}
	{
		size_t clamped_id = m_CurrentLastId % segment_size;
		(m_Buffers.rbegin())->first[clamped_id] = val;
	}
	++m_CurrentLastId;
}

template<typename T, size_t segment_size>
inline void BufferQueue<T, segment_size>::PopBack()
{
	--m_CurrentLastId;
}

template<typename T, size_t segment_size>
inline size_t BufferQueue<T, segment_size>::Size() const
{
	return m_CurrentLastId;
}

template<typename T, size_t segment_size>
inline void BufferQueue<T, segment_size>::Clear()
{
	m_CurrentLastId = 0;
}

template<typename T, size_t segment_size>
inline T& BufferQueue<T, segment_size>::operator[](size_t id)
{
	size_t clamped_id = m_CurrentLastId % segment_size;
	return (m_Buffers.rbegin())->first[clamped_id];
}

template<typename T, size_t segment_size>
inline T const& BufferQueue<T, segment_size>::operator[](size_t id) const
{
	size_t clamped_id = m_CurrentLastId % segment_size;
	return (m_Buffers.rbegin())->first[clamped_id];
}
