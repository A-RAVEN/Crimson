#pragma once
#include <cstdint>
#include <vector>
#include <tuple>
#include "Common.h"
#include <SharedTools/header/uhash.h>

struct InputAssemblyStates
{
public:
	ETopology topology = ETopology::eLineList;

	bool operator==(InputAssemblyStates const& rhs) const
	{
		return topology == rhs.topology;
	}
};

template<>
struct hash_utils::is_contiguously_hashable<InputAssemblyStates> : public std::true_type {};

struct VertexAttribute
{
public:
	uint32_t attributeIndex;
	uint32_t offset;
	VertexInputFormat format;

	bool operator==(VertexAttribute const& rhs) const
	{
		return attributeIndex == rhs.attributeIndex
			&& offset == rhs.offset
			&& format == rhs.format;
	}
};

template<>
struct hash_utils::is_contiguously_hashable<VertexAttribute> : public std::true_type {};

class CVertexInputDescriptor
{
public:
	InputAssemblyStates assemblyStates;
	std::vector<std::tuple<uint32_t, std::vector<VertexAttribute>, bool>> m_PrimitiveDescriptions;

	inline void AddPrimitiveDescriptor(
		uint32_t stride
		, std::vector<VertexAttribute> const& attributes
		, bool perInstance = false
	)
	{
		m_PrimitiveDescriptions.push_back(std::make_tuple(stride, attributes, perInstance));
	}

	bool operator==(CVertexInputDescriptor const& rhs) const
	{
		return assemblyStates == rhs.assemblyStates
			&& m_PrimitiveDescriptions == rhs.m_PrimitiveDescriptions;
	}

	template <class HashAlgorithm>
	friend void hash_append(HashAlgorithm& h, CVertexInputDescriptor const& vertex_input_desc) noexcept
	{
		hash_append(h, vertex_input_desc.assemblyStates);
		for (auto& tup : vertex_input_desc.m_PrimitiveDescriptions)
		{
			hash_append(h, std::get<0>(tup));
			hash_append(h, std::get<1>(tup));
			hash_append(h, std::get<2>(tup));
		}
		hash_append(h, vertex_input_desc.m_PrimitiveDescriptions.size());
	}
};
