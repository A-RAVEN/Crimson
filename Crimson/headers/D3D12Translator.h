#pragma once
#include <headers/D3D12Header.h>
#include <include/Generals.h>
#include <include/PipelineEnums.h>
#include <include/GPUDevice.h>
#include <headers/GeneralDebug.h>
namespace Crimson
{
	static D3D12_COMMAND_LIST_TYPE D3D12_STATIC_COMMAND_LIST_TYPE_TABLE[static_cast<uint32_t>(EExecutionCommandType::E_COMMAND_TYPE_MAX)] =
	{
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		D3D12_COMMAND_LIST_TYPE_COMPUTE,
		D3D12_COMMAND_LIST_TYPE_COPY,
	};

	static D3D12_HEAP_TYPE D3D12_STATIC_MEMORY_TYPE_TO_HEAP_TYPE_TABLE[static_cast<uint32_t>(EMemoryType::E_MEMORY_TYPE_MAX)] =
	{
		D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD,
		D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT,
		D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD,
		D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_READBACK,
	};

	static DXGI_FORMAT D3D12_STATIC_FORMAT_TYPE_TABLE[static_cast<uint32_t>(EFormat::E_FORMAT_MAX)] =
	{
		//R8
		DXGI_FORMAT::DXGI_FORMAT_R8_UNORM,
		DXGI_FORMAT::DXGI_FORMAT_R8_SNORM,
		DXGI_FORMAT::DXGI_FORMAT_R8_UINT,
		DXGI_FORMAT::DXGI_FORMAT_R8_SINT,

		//R8G8
		DXGI_FORMAT::DXGI_FORMAT_R8G8_UNORM,
		DXGI_FORMAT::DXGI_FORMAT_R8G8_SNORM,
		DXGI_FORMAT::DXGI_FORMAT_R8G8_UINT,
		DXGI_FORMAT::DXGI_FORMAT_R8G8_SINT,

		//R8G8B8 Not found in DXGI Definition, use RGBA to replace
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_R8G8B8A8_SNORM,
		DXGI_FORMAT_R8G8B8A8_UINT,
		DXGI_FORMAT_R8G8B8A8_SINT,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,

		//B8G8R8 Not Found
		DXGI_FORMAT_B8G8R8A8_UNORM,
		DXGI_FORMAT_R8G8B8A8_SNORM,
		DXGI_FORMAT_R8G8B8A8_UINT,
		DXGI_FORMAT_R8G8B8A8_SINT,
		DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,

		//R8G8B8A8
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_R8G8B8A8_SNORM,
		DXGI_FORMAT_R8G8B8A8_UINT,
		DXGI_FORMAT_R8G8B8A8_SINT,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,

		//B8G8R8A8
		DXGI_FORMAT_B8G8R8A8_UNORM,
		DXGI_FORMAT_R8G8B8A8_SNORM,
		DXGI_FORMAT_R8G8B8A8_UINT,
		DXGI_FORMAT_R8G8B8A8_SINT,
		DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,

		//R16
		DXGI_FORMAT_R16_UNORM,
		DXGI_FORMAT_R16_SNORM,
		DXGI_FORMAT_R16_UINT,
		DXGI_FORMAT_R16_SINT,
		DXGI_FORMAT_R16_FLOAT,

		//R32
		DXGI_FORMAT_R32_UINT,
		DXGI_FORMAT_R32_SINT,
		DXGI_FORMAT_R32_FLOAT,

		//R64 Not Found In D3D12 Definition
		DXGI_FORMAT_R32_UINT,
		DXGI_FORMAT_R32_SINT,
		DXGI_FORMAT_R32_FLOAT,

		//R32G32B32A32
		DXGI_FORMAT_R32G32B32A32_UINT,
		DXGI_FORMAT_R32G32B32A32_SINT,
		DXGI_FORMAT_R32G32B32A32_FLOAT,

		//D16
		DXGI_FORMAT_D16_UNORM,

		//D32
		DXGI_FORMAT_D32_FLOAT,

		//S8
		DXGI_FORMAT_D24_UNORM_S8_UINT,

		//Depth Stencil
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		DXGI_FORMAT_D32_FLOAT_S8X24_UINT
	};

	static D3D12_PRIMITIVE_TOPOLOGY_TYPE D3D12_STATIC_TOPOLOGY_TYPE_TABLE[static_cast<uint32_t>(ETopology::E_TOPOLOGY_MAX)] =
	{
		//E_TOPOLOGY_POINT_LIST,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT,
		//E_TOPOLOGY_LINE_LIST,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,
		//E_TOPOLOGY_TRIANGLE_LIST,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		//E_TOPOLOGY_TRIANGLE_STRIP,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		//E_TOPOLOGY_PATCH_LIST,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH
	};

	struct D3D12VertexInputDataTypeInfo
	{
		DXGI_FORMAT m_Format;
		uint32_t m_Size;
		uint32_t m_LocationOccupation; // mat4 takes 4 locations
		std::string m_DefaultSematicString;
		D3D12VertexInputDataTypeInfo(DXGI_FORMAT format, uint32_t size, uint32_t location_occupation, std::string default_sematic) :
			m_Format(format),
			m_Size(size),
			m_LocationOccupation(location_occupation),
			m_DefaultSematicString(default_sematic)
		{}
	};

	static D3D12VertexInputDataTypeInfo D3D12_STATIC_DATATYPE_TO_FORMAT_TABLE[static_cast<uint32_t>(EDataType::EDATA_TYPE_MAX)] =
	{
		//scalars
		{DXGI_FORMAT_R32_SINT,				sizeof(int32_t),	1, "EINT"},
		{DXGI_FORMAT_R32_UINT,				sizeof(uint32_t),	1, "EUINT"},
		{DXGI_FORMAT_R32_FLOAT,				sizeof(float),	1, "EFLOAT"},
		//vec
		{DXGI_FORMAT_R32G32_FLOAT,			sizeof(float) * 2,	1, "EVEC2"},
		{DXGI_FORMAT_R32G32B32_FLOAT,		sizeof(float) * 3,	1, "EVEC3"},
		{DXGI_FORMAT_R32G32B32A32_FLOAT,	sizeof(float) * 4,	1, "EVEC4"},
		//ivec
		{DXGI_FORMAT_R32G32_SINT,			sizeof(int32_t) * 2, 1, "EIVEC2"},
		{DXGI_FORMAT_R32G32B32_SINT,		sizeof(int32_t) * 3, 1, "EIVEC3"},
		{DXGI_FORMAT_R32G32B32A32_SINT,		sizeof(int32_t) * 4, 1, "EIVEC4"},
		//uvec
		{DXGI_FORMAT_R32G32_UINT,			sizeof(uint32_t) * 2, 1, "EUVEC2"},
		{DXGI_FORMAT_R32G32B32_UINT,		sizeof(uint32_t) * 3, 1, "EUVEC3"},
		{DXGI_FORMAT_R32G32B32A32_UINT,		sizeof(uint32_t) * 4, 1, "EUVEC4"},
		//mat
		{DXGI_FORMAT_R32G32B32_FLOAT,		sizeof(float) * 3,	3, "EMAT3"},
		{DXGI_FORMAT_R32G32B32A32_FLOAT,	sizeof(float) * 4,	4, "EMAT4"},
		//color
		{DXGI_FORMAT_R8G8B8A8_UNORM,		sizeof(uint8_t) * 4, 1, "ERGBA8_UNORM"},
		{DXGI_FORMAT_R8G8B8A8_SNORM,		sizeof(uint8_t) * 4, 1, "ERGBA8_SNORM"},
	};

	static D3D12_CULL_MODE D3D12_STATIC_CULL_MODE_TABLE[static_cast<uint32_t>(ECullMode::E_CULL_MODE_MAX)] =
	{
		D3D12_CULL_MODE_NONE,
		D3D12_CULL_MODE_FRONT,
		D3D12_CULL_MODE_BACK,
		//Cull front and back not supported in DX12
		D3D12_CULL_MODE_FRONT,
	};
	
	static D3D12_FILL_MODE D3D12_STATIC_FILL_MODE_TABLE[static_cast<uint32_t>(EPolygonMode::E_POLYTON_MODE_MAX)]
	{
		D3D12_FILL_MODE_SOLID,
		D3D12_FILL_MODE_WIREFRAME,
		//draw point is not supported in dx12
		D3D12_FILL_MODE_SOLID,
	};

	static D3D12_COMPARISON_FUNC D3D12_STATIC_COMPARISON_MODE_TABLE[static_cast<uint32_t>(ECompareMode::E_COMPARE_MODE_MAX)]
	{
		D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS,
		D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_GREATER,
	};

	static D3D12_COMMAND_LIST_TYPE D3D12ExecutionCommandTypeToCommandListType(EExecutionCommandType srcType)
	{
		return D3D12_STATIC_COMMAND_LIST_TYPE_TABLE[static_cast<uint32_t>(srcType)];
	}

	static D3D12_HEAP_TYPE D3D12MemoryTypeToHeapType(EMemoryType memoryType)
	{
		return (D3D12_STATIC_MEMORY_TYPE_TO_HEAP_TYPE_TABLE[static_cast<uint32_t>(memoryType)]);
	}

	static DXGI_FORMAT D3D12FormatType(EFormat format)
	{
		CRIM_ASSERT(format != EFormat::E_FORMAT_B8G8R8A8_SNORM && format != EFormat::E_FORMAT_B8G8R8_SNORM, "DXGI does not support BGR(A)_SNORM, Fallback to RGB(A)_SNORM");
		CRIM_ASSERT(format != EFormat::E_FORMAT_B8G8R8A8_UINT && format != EFormat::E_FORMAT_B8G8R8_UINT, "DXGI does not support BGR(A)_UINT, Fallback to RGB(A)_UINT");
		CRIM_ASSERT(format != EFormat::E_FORMAT_B8G8R8A8_SINT && format != EFormat::E_FORMAT_B8G8R8_SINT, "DXGI does not support BGR(A)_SINT, Fallback to RGB(A)_SINT");
		CRIM_ASSERT(format != EFormat::E_FORMAT_R64_SFLOAT && format != EFormat::E_FORMAT_R64_UINT && format != EFormat::E_FORMAT_R64_SINT, "DXGI does not support R64 format, Fallback to R32 format");
		return D3D12_STATIC_FORMAT_TYPE_TABLE[static_cast<uint32_t>(format)];
	}

	inline static D3D12_PRIMITIVE_TOPOLOGY_TYPE D3D12TopologyType(ETopology topology)
	{
		CRIM_ASSERT(topology != ETopology::E_TOPOLOGY_TRIANGLE_STRIP, "DXGI does not support Triangle Strip Topology, Fallback to Triangles");
		return D3D12_STATIC_TOPOLOGY_TYPE_TABLE[static_cast<uint32_t>(topology)];
	}

	inline static D3D12VertexInputDataTypeInfo const& D3D12VertexInputDataType(EDataType dataType)
	{
		return D3D12_STATIC_DATATYPE_TO_FORMAT_TABLE[static_cast<uint32_t>(dataType)];
	}

	inline static D3D12_CULL_MODE D3D12CullMode(ECullMode cull_mode)
	{
		CRIM_ASSERT(cull_mode != ECullMode::E_CULL_FRONT_BACK, "DXGI does not support Cull Mode Front Back, Fallback to Cull Front");
		return D3D12_STATIC_CULL_MODE_TABLE[static_cast<uint32_t>(cull_mode)];
	}

	inline static D3D12_FILL_MODE D3D12FillMode(EPolygonMode polygon_mode)
	{
		CRIM_ASSERT(polygon_mode != EPolygonMode::E_POLYGON_MODE_POINT, "DXGI does not support Polygon Mode Point, Fallback to Polygon Mode Fill");
		return D3D12_STATIC_FILL_MODE_TABLE[static_cast<uint32_t>(polygon_mode)];
	}

	inline static D3D12_COMPARISON_FUNC D3D12ComparisonFunc(ECompareMode compare_mode)
	{
		return D3D12_STATIC_COMPARISON_MODE_TABLE[static_cast<uint32_t>(compare_mode)];
	}
}