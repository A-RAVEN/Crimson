#pragma once
#include <headers/VulkanHeader.h>
#include <headers/vk_mem_alloc.h>
#include <unordered_map>
#include <include/Generals.h>
#include <include/Pipeline.h>

namespace Crimson
{
	static VkFormat VULKAN_STATIC_FORMAT_TABLE[] =
	{
		//R8
		VK_FORMAT_R8_UNORM,
		VK_FORMAT_R8_SNORM,
		VK_FORMAT_R8_UINT,
		VK_FORMAT_R8_SINT,

		//R8G8
		VK_FORMAT_R8G8_UNORM,
		VK_FORMAT_R8G8_SNORM,
		VK_FORMAT_R8G8_UINT,
		VK_FORMAT_R8G8_SINT,

		//R8G8B8
		VK_FORMAT_R8G8B8_UNORM,
		VK_FORMAT_R8G8B8_SNORM,
		VK_FORMAT_R8G8B8_UINT,
		VK_FORMAT_R8G8B8_SINT,
		VK_FORMAT_R8G8B8_SRGB,

		//B8G8R8
		VK_FORMAT_B8G8R8_UNORM,
		VK_FORMAT_B8G8R8_SNORM,
		VK_FORMAT_B8G8R8_UINT,
		VK_FORMAT_B8G8R8_SINT,
		VK_FORMAT_B8G8R8_SRGB,

		//R8G8B8A8
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_FORMAT_R8G8B8A8_SNORM,
		VK_FORMAT_R8G8B8A8_UINT,
		VK_FORMAT_R8G8B8A8_SINT,
		VK_FORMAT_R8G8B8A8_SRGB,

		//B8G8R8A8
		VK_FORMAT_B8G8R8A8_UNORM,
		VK_FORMAT_B8G8R8A8_SNORM,
		VK_FORMAT_B8G8R8A8_UINT,
		VK_FORMAT_B8G8R8A8_SINT,
		VK_FORMAT_B8G8R8A8_SRGB,

		//R16
		VK_FORMAT_R16_UNORM,
		VK_FORMAT_R16_SNORM,
		VK_FORMAT_R16_UINT,
		VK_FORMAT_R16_SINT,
		VK_FORMAT_R16_SFLOAT,

		//R32
		VK_FORMAT_R32_UINT,
		VK_FORMAT_R32_SINT,
		VK_FORMAT_R32_SFLOAT,

		//R64
		VK_FORMAT_R64_UINT,
		VK_FORMAT_R64_SINT,
		VK_FORMAT_R64_SFLOAT,

		//R32G32B32A32
		VK_FORMAT_R32G32B32A32_UINT,
		VK_FORMAT_R32G32B32A32_SINT,
		VK_FORMAT_R32G32B32A32_SFLOAT,

		//D16
		VK_FORMAT_D16_UNORM,

		//D32
		VK_FORMAT_D32_SFLOAT,

		//S8
		VK_FORMAT_S8_UINT,

		//Depth Stencil
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D32_SFLOAT_S8_UINT,
	};

	static VkBufferUsageFlagBits VULKAN_STATIC_BUFFER_USAGE_TABLE[static_cast<uint16_t>(EBufferUsage::E_BUFFER_USAGE_MAX)] =
	{
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
		VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
		VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT,
		VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
	};

	static VkImageUsageFlagBits VULKAN_STATIC_IMAGE_USAGE_TABLE[] =
	{
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_IMAGE_USAGE_STORAGE_BIT,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
	};

	static VmaMemoryUsage VMA_STATIC_MEMORY_USAGE_TABLE[] =
	{
		VMA_MEMORY_USAGE_CPU_ONLY,
		VMA_MEMORY_USAGE_GPU_ONLY,
		VMA_MEMORY_USAGE_CPU_TO_GPU,
		VMA_MEMORY_USAGE_GPU_TO_CPU
	};

	static VkShaderStageFlagBits VULKAN_STATIC_SHADER_STAGE_TABLE[] =
	{
		VK_SHADER_STAGE_VERTEX_BIT,
		VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
		VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
		VK_SHADER_STAGE_GEOMETRY_BIT,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		VK_SHADER_STAGE_COMPUTE_BIT,
		VK_SHADER_STAGE_TASK_BIT_NV,
		VK_SHADER_STAGE_MESH_BIT_NV,
		VK_SHADER_STAGE_RAYGEN_BIT_NV,
		VK_SHADER_STAGE_ANY_HIT_BIT_NV,
		VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV,
		VK_SHADER_STAGE_MISS_BIT_NV,
		VK_SHADER_STAGE_INTERSECTION_BIT_NV,

		VK_SHADER_STAGE_ALL_GRAPHICS,
	};

	static VkPipelineStageFlagBits VULKAN_STATIC_SHADER_TO_PIPELINE_STAGE_TABLE[] =
	{
		VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
		VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT,
		VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT,
		VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV,
		VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
	};
	
	static VkDescriptorType VULKAN_STATIC_DESCRIPTOR_TYPE_TABLE[static_cast<size_t>(EShaderResourceType::E_SHADER_RESOURCE_TYPE_MAX)] =
	{
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
		VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV,
		VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
		VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
	};

	static VkSamplerAddressMode VULKAN_STATIC_SAMPLER_ADDRESS_MODE_TABLE[static_cast<size_t>(EAddrMode::E_ADDR_MAX)] =
	{
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
		VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
	};

	static VkFilter VULKAN_STATIC_FILTER_MODE_TABLE[static_cast<size_t>(EFilterMode::E_FILTER_MODE_MAX)]
	{
		VK_FILTER_NEAREST,
		VK_FILTER_LINEAR
	};

	static VkSamplerMipmapMode VULKAN_STATIC_MIPMAP_MODE_TABLE[static_cast<size_t>(EFilterMode::E_FILTER_MODE_MAX)]
	{
		VK_SAMPLER_MIPMAP_MODE_NEAREST,
		VK_SAMPLER_MIPMAP_MODE_LINEAR,
	};

	static VkAccessFlags VULKAN_STATIC_IMAGE_LAYOUT_TO_ACCESS_FLAGS_TABLE[9] =
	{
		0,
		VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_ACCESS_TRANSFER_READ_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		0,
	};

	static VkGeometryFlagBitsNV VULKAN_STATIC_GEOMETRY_FLAG_TABLE[static_cast<uint32_t>(EGeometryFlags::E_GEOMETRY_MAX)] =
	{
		VK_GEOMETRY_OPAQUE_BIT_NV,
		VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_NV,
	};

	struct VulkanVertexInputDataTypeInfo
	{
		VkFormat m_Format;
		uint32_t m_Size;
		uint32_t m_LocationOccupation; // mat4 takes 4 locations
		VulkanVertexInputDataTypeInfo(VkFormat format, uint32_t size, uint32_t location_occupation) :
			m_Format(format),
			m_Size(size),
			m_LocationOccupation(location_occupation)
		{}
	};

	static VulkanVertexInputDataTypeInfo VULKAN_STATIC_DATA_TYPE_TO_VERTEX_INPUT_DATA_TABLE[static_cast<size_t>(EDataType::EDATA_TYPE_MAX)] =
	{
		//scalar series
		{VK_FORMAT_R32_SINT,			sizeof(int32_t),	1},
		{VK_FORMAT_R32_UINT,			sizeof(uint32_t),	1},
		{VK_FORMAT_R32_SFLOAT,			sizeof(float),		1},
		//vec series
		{VK_FORMAT_R32G32_SFLOAT,		sizeof(float) * 2,	1},
		{VK_FORMAT_R32G32B32_SFLOAT,	sizeof(float) * 3,	1},
		{VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 4,	1},
		//ivec series
		{VK_FORMAT_R32G32_SINT,			sizeof(int32_t) * 2, 1},
		{VK_FORMAT_R32G32B32_SINT,		sizeof(int32_t) * 3, 1},
		{VK_FORMAT_R32G32B32A32_SINT,	sizeof(int32_t) * 4, 1},
		//uvec series
		{VK_FORMAT_R32G32_UINT,			sizeof(uint32_t) * 2, 1},
		{VK_FORMAT_R32G32B32_UINT,		sizeof(uint32_t) * 3, 1},
		{VK_FORMAT_R32G32B32A32_UINT,	sizeof(uint32_t) * 4, 1},
		//mat series
		{VK_FORMAT_R32G32B32_SFLOAT,	sizeof(float) * 3,	3},
		{VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 4,	4},
		//color
		{VK_FORMAT_R8G8B8A8_UNORM,		sizeof(uint8_t) * 4, 1},
		{VK_FORMAT_R8G8B8A8_SNORM,		sizeof(uint8_t) * 4, 1}
	};

	static VkImageAspectFlags VULKAN_STATIC_IMAGE_ASPECT_TYPE_TABLE[static_cast<size_t>(EViewAsType::E_VIEW_AS_TYPE_MAX)] =
	{
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_ASPECT_DEPTH_BIT,
		VK_IMAGE_ASPECT_STENCIL_BIT,
		VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
	};

	static VkPrimitiveTopology VULKAN_STATIC_TOPOLOTY_TABLE[static_cast<size_t>(ETopology::E_TOPOLOGY_MAX)] =
	{
		VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
		VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
		VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
	};

	static VkPolygonMode VULKAN_STATIC_POLYGON_MODE_TABLE[static_cast<size_t>(EPolygonMode::E_POLYTON_MODE_MAX)] =
	{
		VK_POLYGON_MODE_FILL,
		VK_POLYGON_MODE_LINE,
		VK_POLYGON_MODE_POINT,
	};

	static VkCullModeFlagBits VULKAN_STATIC_CULL_MODE_TABLE[static_cast<size_t>(ECullMode::E_CULL_MODE_MAX)] =
	{
		VK_CULL_MODE_NONE,
		VK_CULL_MODE_FRONT_BIT,
		VK_CULL_MODE_BACK_BIT,
		VK_CULL_MODE_FRONT_AND_BACK
	};

	static VkCompareOp VULKAN_STATIC_COMPARE_OP_TABLE[static_cast<size_t>(ECompareMode::E_COMPARE_MODE_MAX)] =
	{
		VK_COMPARE_OP_LESS,
		VK_COMPARE_OP_LESS_OR_EQUAL,
		VK_COMPARE_OP_GREATER
	};

	static VkBlendFactor VULKAN_STATIC_BLEND_FACTOR_TABLE[static_cast<size_t>(EBlendFactor::E_BLEND_FACTOR_MAX)] =
	{
		VK_BLEND_FACTOR_ONE,
		VK_BLEND_FACTOR_ZERO,
		VK_BLEND_FACTOR_SRC_ALPHA
	};

	static VkBlendOp VULKAN_STATIC_BLEND_OP_TABLE[static_cast<size_t>(EBlendOp::E_BLEND_OP_MAX)] =
	{
		VK_BLEND_OP_ADD,
		VK_BLEND_OP_MINUS_EXT,
		VK_BLEND_OP_SUBTRACT,
		VK_BLEND_OP_MULTIPLY_EXT,
	};

	static VkBuildAccelerationStructureFlagBitsNV VULKAN_BUILD_ACCEL_STRUCT_FLAG_NV_TABLE[static_cast<size_t>(EBuildAccelerationStructureFlags::E_BUILD_ACCELERATION_STRUCTURE_MAX)] =
	{
		VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV,
		VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_NV,
		VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV,
		VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV,
		VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_NV,
	};

	static inline VkFormat TranslateImageFormatToVulkan(EFormat format)
	{
		return VULKAN_STATIC_FORMAT_TABLE[static_cast<uint32_t>(format)];
	}

	static VkBufferUsageFlags TranslateBufferUsageFlagsToVulkan(std::vector<EBufferUsage> const& usages)
	{
		VkBufferUsageFlags return_val = 0;
		for (auto usage : usages)
		{
			return_val |= VULKAN_STATIC_BUFFER_USAGE_TABLE[static_cast<uint32_t>(usage)];
		}
		return return_val;
	}

	static VkImageUsageFlags TranslateImageUsageFlagsToVulkan(std::vector<EImageUsage> const& usages)
	{
		VkImageUsageFlags return_val = 0;
		for (auto usage : usages)
		{
			return_val |= VULKAN_STATIC_IMAGE_USAGE_TABLE[static_cast<uint32_t>(usage)];
		}
		return return_val;
	}

	static inline VmaMemoryUsage TranslateMemoryUsageToVMA(EMemoryType memory_type)
	{
		return VMA_STATIC_MEMORY_USAGE_TABLE[static_cast<uint32_t>(memory_type)];
	}

	static VkSampleCountFlagBits TranslateSampleCountToVulkan(uint32_t sample_count)
	{
		switch (sample_count)
		{
		case 1:
			return VK_SAMPLE_COUNT_1_BIT;
		case 2:
			return VK_SAMPLE_COUNT_2_BIT;
		case 4:
			return VK_SAMPLE_COUNT_4_BIT;
		case 8:
			return VK_SAMPLE_COUNT_8_BIT;
		case 16:
			return VK_SAMPLE_COUNT_16_BIT;
		case 32:
			return VK_SAMPLE_COUNT_32_BIT;
		case 64:
			return VK_SAMPLE_COUNT_64_BIT;
		}
		return VK_SAMPLE_COUNT_1_BIT;
	}

	static inline VkShaderStageFlagBits TranslateShaderTypeToVulkan(EShaderType shader_type)
	{
		return VULKAN_STATIC_SHADER_STAGE_TABLE[static_cast<uint32_t>(shader_type)];
	}

	static inline bool IsNormalGraphicsShaderGroup(EShaderType shader_type)
	{
		switch (shader_type) 
		{
		case EShaderType::E_SHADER_TYPE_VERTEX:
		case EShaderType::E_SHADER_TYPE_TESSCTR:
		case EShaderType::E_SHADER_TYPE_TESSEVL:
		case EShaderType::E_SHADER_TYPE_GEOMETRY:
		case EShaderType::E_SHADER_TYPE_FRAGMENT:
			return true;
		}
		return false;
	}

	static inline bool IsMeshShaderGroup(EShaderType shader_type)
	{
		switch (shader_type)
		{
		case EShaderType::E_SHADER_TYPE_TASK_NV:
		case EShaderType::E_SHADER_TYPE_MESH_NV:
		case EShaderType::E_SHADER_TYPE_FRAGMENT:
			return true;
		}
		return false;
	}

	static inline bool IsRayTracingShaderGroup(EShaderType shader_type)
	{
		return (shader_type >= EShaderType::E_SHADER_TYPE_RAYGEN_NV) && (shader_type < EShaderType::E_SHADER_TYPE_MAX);
	}

	static inline VkPipelineStageFlagBits TranslateShaderTypeToVulkanPipelineStage(EShaderType shader_type)
	{
		return VULKAN_STATIC_SHADER_TO_PIPELINE_STAGE_TABLE[static_cast<uint32_t>(shader_type)];
	}

	static inline VkDescriptorType TranslateShaderResourceTypeToVulkan(EShaderResourceType resource_type)
	{
		return VULKAN_STATIC_DESCRIPTOR_TYPE_TABLE[static_cast<uint32_t>(resource_type)];
	}

	static inline VulkanVertexInputDataTypeInfo const& TranslateDataTypeToVulkanVertexInputDataTypeInfo(EDataType data_type)
	{
		return VULKAN_STATIC_DATA_TYPE_TO_VERTEX_INPUT_DATA_TABLE[static_cast<size_t>(data_type)];
	}

	static inline VkPrimitiveTopology TranslateTopologyToVulkan(ETopology topology)
	{
		return VULKAN_STATIC_TOPOLOTY_TABLE[static_cast<size_t>(topology)];
	}

	static inline VkPolygonMode TranslatePolygonModeToVulkan(EPolygonMode polygon_mode)
	{
		return VULKAN_STATIC_POLYGON_MODE_TABLE[static_cast<size_t>(polygon_mode)];
	}

	static inline VkCullModeFlagBits TranslateCullModeToVulkan(ECullMode cull_mode)
	{
		return VULKAN_STATIC_CULL_MODE_TABLE[static_cast<size_t>(cull_mode)];
	}

	static inline VkCompareOp TranslateCompareModeToVulkan(ECompareMode compare_mode)
	{
		return VULKAN_STATIC_COMPARE_OP_TABLE[static_cast<size_t>(compare_mode)];
	}

	static inline VkBlendFactor TranslateBlendFactorToVulkan(EBlendFactor blend_factor)
	{
		return VULKAN_STATIC_BLEND_FACTOR_TABLE[static_cast<size_t>(blend_factor)];
	}

	static inline VkBlendOp TranslateBlendOpToVulkan(EBlendOp blend_op)
	{
		return VULKAN_STATIC_BLEND_OP_TABLE[static_cast<size_t>(blend_op)];
	}

	static inline VkImageAspectFlags TranslateViewAsTypeToVulkanAspectFlags(EViewAsType view_as_type)
	{
		return VULKAN_STATIC_IMAGE_ASPECT_TYPE_TABLE[static_cast<size_t>(view_as_type)];
	}

	static inline VkSamplerAddressMode TranslateSamplerAddressModeToVulkan(EAddrMode addr_mode)
	{
		return VULKAN_STATIC_SAMPLER_ADDRESS_MODE_TABLE[static_cast<size_t>(addr_mode)];
	}

	static inline VkFilter TranslateFilterModeToVulkanFilter(EFilterMode filter_mode)
	{
		return VULKAN_STATIC_FILTER_MODE_TABLE[static_cast<size_t>(filter_mode)];
	}

	static inline VkSamplerMipmapMode TranslateFilterModeToVulkanMipMapMode(EFilterMode filter_mode)
	{
		return VULKAN_STATIC_MIPMAP_MODE_TABLE[static_cast<size_t>(filter_mode)];
	}

	static inline VkAccessFlags VkTranslateLayoutToAccessFlags(VkImageLayout layout)
	{
		if (layout < 9)
		{
			return VULKAN_STATIC_IMAGE_LAYOUT_TO_ACCESS_FLAGS_TABLE[layout];
		}
		switch (layout)
		{
		case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
			return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}
	}

	static inline VkGeometryFlagBitsNV TranslateGeometryFlagToVulkan(EGeometryFlags flag)
	{
		return VULKAN_STATIC_GEOMETRY_FLAG_TABLE[static_cast<uint8_t>(flag)];
	}

	static inline VkImageLayout TranslateShaderResourceTypeToVulkanImageLayout(EShaderResourceType resource_type, EFormat image_format)
	{
		bool is_depth_stencil = IsDepthStencilFormat(image_format);
		switch (resource_type)
		{
		case Crimson::EShaderResourceType::E_SHADER_IMAGE_SAMPLER:
			return is_depth_stencil ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			break;
		case Crimson::EShaderResourceType::E_SHADER_STORAGE_IMAGE:
			return VK_IMAGE_LAYOUT_GENERAL;
			break;
		case Crimson::EShaderResourceType::E_SHADER_TYPE_INPUT_ATTACHMENT:
			return is_depth_stencil ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			break;
		default:
			break;
		}
		return VK_IMAGE_LAYOUT_GENERAL;
	}

	static inline VkBuildAccelerationStructureFlagsNV TranslateBuildAccelStructureFlags(std::vector<EBuildAccelerationStructureFlags> const& flags)
	{
		VkBuildAccelerationStructureFlagsNV return_val = 0;
		for (auto flag : flags)
		{
			return_val |= VULKAN_BUILD_ACCEL_STRUCT_FLAG_NV_TABLE[static_cast<size_t>(flag)];
		}
		return return_val;
	}
}