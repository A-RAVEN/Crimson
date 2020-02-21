#pragma once
#include <stdint.h>

namespace Crimson
{
	enum class EShaderResourceType : uint8_t
	{
		E_SHADER_UNIFORM_BUFFER = 0,
		E_SHADER_IMAGE_SAMPLER,
		E_SHADER_STORAGE_IMAGE,
		E_SHADER_TYPE_STORAGE_BUFFER,
		E_SHADER_TYPE_INPUT_ATTACHMENT,
		E_SHADER_ACCEL_STRUCT_NV,
		E_SHADER_BINDING_MAX,
	};

	enum class EShaderType : uint8_t
	{
		E_SHADER_TYPE_VERTEX = 0,
		E_SHADER_TYPE_TESSCTR,
		E_SHADER_TYPE_TESSEVL,
		E_SHADER_TYPE_GEOMETRY,
		E_SHADER_TYPE_FRAGMENT,
		E_SHADER_TYPE_COMPUTE,
		//nvidia mesh shader
		E_SHADER_TYPE_TASK_NV,
		E_SHADER_TYPE_MESH_NV,
		//nvidia ray tracing shader
		E_SHADER_TYPE_RAYGEN_NV,
		E_SHADER_TYPE_ANYHIT_NV,
		E_SHADER_TYPE_CLOSEHIT_NV,
		E_SHADER_TYPE_MISS_NV,
		E_SHADER_TYPE_INTERSECTION_NV,
		E_SHADER_TYPE_MAX,
	};

	enum class EBufferUniformType
	{
		E_BUFFER_UNIFORM_TYPE_UNIFORM_BUFFER = 0,
		E_BUFFER_UNIFORM_TYPE_STORAGE_BUFFER,
		E_BUFFER_UNIFORM_TYPE_MAX
	};



	enum class EVertexInputMode : uint8_t
	{
		E_VERTEX_INPUT_PER_VERTEX = 0,
		E_VERTEX_INPUT_PER_INSTANCE,
		E_VERTEX_INPUT_MODE_MAX,
	};

	enum class EDataType : uint16_t
	{
		//scalars
		EINT = 0,
		EUINT,
		EFLOAT,
		//vec
		EVEC2,
		EVEC3,
		EVEC4,
		//ivec
		EIVEC2,
		EIVEC3,
		EIVEC4,
		//uvec
		EUVEC2,
		EUVEC3,
		EUVEC4,
		//mat
		EMAT3,
		EMAT4,
		//color
		ERGBA8_UNORM,
		ERGBA8_SNORM,
		EDATA_TYPE_MAX,
	};

	enum class EBlendFactor : uint8_t
	{
		E_ONE = 0,
		E_ZERO,
		E_SRC_ALPHA,
		E_DST_ALPHA,
		E_ONE_MINUS_SRC_ALPHA,
		E_ONE_MINUS_DST_ALPHA,
		E_BLEND_FACTOR_MAX,
	};

	enum class EBlendOp : uint8_t
	{
		E_ADD = 0,
		E_MINUS,
		E_SUBTRACT,
		E_MULTIPLY,
		E_BLEND_OP_MAX
	};

	enum class EDepthTestRule : uint8_t
	{
		E_DEPTH_TEST_DISABLED = 0,
		E_DEPTH_TEST_ENABLED,
		E_DEPTH_TEST_READ_ONLY,
		E_DEPTH_TEST_MAX
	};

	enum class ECompareMode : uint8_t
	{
		E_COMPARE_MODE_LESS = 0,
		E_COMPARE_MODE_LESS_OR_EQUAL,
		E_COMPARE_MODE_GREATER,
		E_COMPARE_MODE_MAX
	};

	enum class EStencilRule : uint8_t
	{
		E_STENCIL_DISABLED = 0,
		E_STENCIL_WRITE,
		E_STENCIL_TEST,
		E_STENCIL_INVTEST,
		E_STENCIL_MAX
	};

	enum class EIndexType : uint8_t
	{
		E_INDEX_TYPE_16 = 0,
		E_INDEX_TYPE_32,
		E_INDEX_TYPE_MAX
	};

	static inline bool IsBufferResourceType(EShaderResourceType resource_type)
	{
		switch (resource_type)
		{
		case EShaderResourceType::E_SHADER_UNIFORM_BUFFER:
		case EShaderResourceType::E_SHADER_TYPE_STORAGE_BUFFER:
			return true;
		}
		return false;
	}

	static inline bool IsImageResourceType(EShaderResourceType resource_type)
	{
		switch (resource_type)
		{
		case EShaderResourceType::E_SHADER_IMAGE_SAMPLER:
		case EShaderResourceType::E_SHADER_STORAGE_IMAGE:
		case EShaderResourceType::E_SHADER_TYPE_INPUT_ATTACHMENT:
			return true;
		}
		return false;
	}
}