#pragma once
#include <cstdint>
#include <vector>
#include <tuple>
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

	enum class VertexInputFormat
	{
		eR32_SFloat = 0,
		eR32G32_SFloat,
		eR32G32B32_SFloat,
		eR32G32B32A32_SFloat,
		eR32_UInt,
		eR32_SInt,
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
		VertexInputFormat format;
	};

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
	};

	//class CVertexInputInstanceDescriptor
	//{
	//	std::vector<std::tuple<uint32_t, std::vector<VertexAttribute>>> m_PrimitiveDescriptions_Instanced;

	//	inline void AddInstanceDescriptor(
	//		uint32_t stride
	//		, std::vector<VertexAttribute> const& attributes)
	//	{
	//		m_PrimitiveDescriptions_Instanced.push_back(std::make_tuple(stride, attributes));
	//	}
	//};
}