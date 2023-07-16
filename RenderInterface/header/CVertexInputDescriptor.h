#pragma once
#include <cstdint>
#include <vector>
namespace graphics_backend
{
	enum class ETopology
	{
		eTriangleList = 0,
		eTriangleStrip,
		ePointList,
		eLineList,
		eLineStrip,
	};

	struct InputAssemblyStates
	{
	public:
		ETopology topology = ETopology::eLineList;
	};

	struct VertexAttribute
	{
	public:
		uint32_t attributeIndex;
		uint32_t offset;
		uint32_t size;
	};

	class CVertexInputDescriptor
	{
	public:
		InputAssemblyStates m_AssemblyStates;
		std::vector<std::tuple<uint32_t, std::vector<VertexAttribute>, bool>> m_PrimitiveDescriptions;

		inline void AddPrimitiveDescriptor(
			uint32_t stride
			, std::vector<VertexAttribute> const& attributes
			, bool perInstance = false
		)
		{
			m_PrimitiveDescriptions.push_back(std::make_tuple(stride, attributes, perInstance));
		}
	};

	class CVertexInputInstanceDescriptor
	{
		std::vector<std::tuple<uint32_t, std::vector<VertexAttribute>>> m_PrimitiveDescriptions_Instanced;
	};
}