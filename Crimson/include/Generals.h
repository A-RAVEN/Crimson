#pragma once
#include <stdint.h>
#include <memory>
#include <vector>

namespace Crimson
{
	class IObjectManager;
	class IObject;
	class IGPUBuffer;
	class IGPUImage;
	class GraphicsPipeline;

	enum class EFormat : uint32_t
	{
		//R8
		E_FORMAT_R8_UNORM = 0,
		E_FORMAT_R8_SNORM,
		E_FORMAT_R8_UINT,
		E_FORMAT_R8_SINT,

		//R8G8
		E_FORMAT_R8G8_UNORM,
		E_FORMAT_R8G8_SNORM,
		E_FORMAT_R8G8_UINT,
		E_FORMAT_R8G8_SINT,

		//R8G8B8
		E_FORMAT_R8G8B8_UNORM,
		E_FORMAT_R8G8B8_SNORM,
		E_FORMAT_R8G8B8_UINT,
		E_FORMAT_R8G8B8_SINT,
		E_FORMAT_R8G8B8_SRGB,

		//B8G8R8
		E_FORMAT_B8G8R8_UNORM,
		E_FORMAT_B8G8R8_SNORM,
		E_FORMAT_B8G8R8_UINT,
		E_FORMAT_B8G8R8_SINT,
		E_FORMAT_B8G8R8_SRGB,

		//R8G8B8A8
		E_FORMAT_R8G8B8A8_UNORM,
		E_FORMAT_R8G8B8A8_SNORM,
		E_FORMAT_R8G8B8A8_UINT,
		E_FORMAT_R8G8B8A8_SINT,
		E_FORMAT_R8G8B8A8_SRGB,

		//B8G8R8A8
		E_FORMAT_B8G8R8A8_UNORM,
		E_FORMAT_B8G8R8A8_SNORM,
		E_FORMAT_B8G8R8A8_UINT,
		E_FORMAT_B8G8R8A8_SINT,
		E_FORMAT_B8G8R8A8_SRGB,

		//R16
		E_FORMAT_R16_UNORM,
		E_FORMAT_R16_SNORM,
		E_FORMAT_R16_UINT,
		E_FORMAT_R16_SINT,
		E_FORMAT_R16_SFLOAT,

		//R32
		E_FORMAT_R32_UINT,
		E_FORMAT_R32_SINT,
		E_FORMAT_R32_SFLOAT,

		//R64
		E_FORMAT_R64_UINT,
		E_FORMAT_R64_SINT,
		E_FORMAT_R64_SFLOAT,

		//R32G32B32A32
		E_FORMAT_R32G32B32A32_UINT,
		E_FORMAT_R32G32B32A32_SINT,
		E_FORMAT_R32G32B32A32_SFLOAT,

		//D16
		E_FORMAT_D16_UNORM,

		//D32
		E_FORMAT_D32_SFLOAT,

		//S8
		E_FORMAT_S8_UINT,

		//Depth Stencil
		E_FORMAT_D16_UNORM_S8_UINT,
		E_FORMAT_D24_UNORM_S8_UINT,
		E_FORMAT_D32_SFLOAT_S8_UINT,

		//
		E_FORMAT_MAX
	};

	static inline bool IsColorFormat(EFormat format)
	{
		return format <= EFormat::E_FORMAT_R32G32B32A32_SFLOAT;
	}

	static inline bool IsDepthStencilFormat(EFormat format)
	{
		return format >= EFormat::E_FORMAT_D16_UNORM_S8_UINT && format < EFormat::E_FORMAT_MAX;
	}

	static inline bool IsStencilOnlyFormat(EFormat format)
	{
		return format >= EFormat::E_FORMAT_S8_UINT && format <= EFormat::E_FORMAT_S8_UINT;
	}

	static inline bool IsDepthOnlyFormat(EFormat format)
	{
		return format >= EFormat::E_FORMAT_D16_UNORM && format <= EFormat::E_FORMAT_D32_SFLOAT;
	}

	enum class ECommandType
	{
		E_COMMAND_TYPE_GRAPHICS,
		E_COMMAND_TYPE_COMPUTE,
		E_COMMAND_TYPE_TRANSFER_DEDICATE,
		E_COMMAND_TYPE_MAX
	};

	enum class EObjectType : uint16_t
	{
		E_OBJ_TYPE_BUFFER = 0,
		E_OBJ_TYPE_TEXTURE,
		E_OBJ_TYPE_PIPELINE,
		E_OBJ_TYPE_RENDERPASS,
		E_OBJ_TYPE_FRAMEBUFFER,
		E_OBJ_TYPE_MAX
	};

	enum class EBufferUsage : uint16_t
	{
		E_BUFFER_USAGE_COPY_SRC = 0,
		E_BUFFER_USAGE_COPY_DST,
		E_BUFFER_USAGE_VERTEX,
		E_BUFFER_USAGE_INDICIES,
		E_BUFFER_USAGE_UNIFORM,
		E_BUFFER_USAGE_STORAGE,
		E_BUFFER_USAGE_INDIRECT_DRAW,
		E_BUFFER_USAGE_RAYTRACING_NV,
		E_BUFFER_USAGE_MAX,
	};

	enum class EImageUsage : uint16_t
	{
		E_IMAGE_USAGE_COPY_SRC = 0,
		E_IMAGE_USAGE_COPY_DST,
		E_IMAGE_USAGE_SAMPLE,
		E_IMAGE_USAGE_STORAGE,
		E_IMAGE_USAGE_COLOR_ATTACHMENT,
		E_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT,
		E_IMAGE_USAGE_MAX
	};

	enum class EMemoryType : uint8_t
	{
		//Staging buffers, quick access on CPU, slow on GPU
		E_MEMORY_TYPE_HOST = 0,
		//May be invisible to host, GPU access fast
		E_MEMORY_TYPE_DEVICE,
		//May be accessed fast both by CPU and GPU
		E_MEMORY_TYPE_HOST_TO_DEVICE,
		//Read back from GPU
		E_MEMORY_TYPE_DEVICE_TO_HOST,
		E_MEMORY_TYPE_MAX,
	};

	enum class EAttachmentClearType : uint8_t
	{
		E_ATTACHMENT_CLEAR_ZEROS = 0,
		E_ATTACHMENT_CLEAR_ONES,
		E_ATTACHMENT_NOT_CLEAR,
	};

	enum class ECullMode : uint8_t
	{
		E_CULL_NONE = 0,
		E_CULL_FRONT,
		E_CULL_BACK,
		E_CULL_FRONT_BACK,
		E_CULL_MODE_MAX
	};

	enum class EPolygonMode : uint8_t
	{
		E_POLYGON_MODE_FILL = 0,
		E_POLYGON_MODE_LINE,
		E_POLYGON_MODE_POINT,
		E_POLYTON_MODE_MAX,
	};

	enum class ETopology : uint8_t
	{
		E_TOPOLOGY_POINT_LIST = 0,
		E_TOPOLOGY_LINE_LIST,
		E_TOPOLOGY_TRIANGLE_LIST,
		E_TOPOLOGY_TRIANGLE_STRIP,
		E_TOPOLOGY_PATCH_LIST,
		E_TOPOLOGY_MAX,
	};

	enum class EViewAsType : uint32_t
	{
		//E_VIEW_AS_DEFAULT = 0,
		E_VIEW_AS_COLOR = 0,
		E_VIEW_AS_DEPTH,
		E_VIEW_AS_STENCIL,
		E_VIEW_AS_DEPTH_STENCIL,
		E_VIEW_AS_TYPE_MAX
	};

	enum class EFilterMode : uint8_t
	{
		E_FILTER_MODE_NEAREST = 0,
		E_FILTER_MODE_LINEAR,
		E_FILTER_MODE_MAX
	};

	enum class EAddrMode : uint8_t
	{
		E_ADDR_MODE_CLAMP_TO_EDGE = 0,
		E_ADDR_MODE_CLAMP_TO_BORDER,
		E_ADDR_MODE_REPEAT,
		E_ADDR_MIRRORED_REPEAT,
		E_ADDR_MAX
	};

	class IObjectManager
	{
	public:
		//virtual void HandleDisposedBuffer(IGPUBuffer* buffer) {};
		//virtual void HandleDisposedImage(IGPUImage* buffer) {};
	};

	class IObject
	{
	public:
		IObject() {}
		virtual void Dispose() {}
	protected:
		virtual ~IObject() {}
	};

	class BufferRange
	{
	public:
		uint64_t m_Offset;
		uint64_t m_Size;
		BufferRange(uint64_t offset = 0u, uint64_t size = 0u) :
			m_Offset(offset),
			m_Size(size)
		{}
	};

	class IGPUBuffer : public IObject
	{
	public:
		virtual void Dispose() override {}
		virtual uint8_t* GetMappedPointer() = 0;
		virtual void UnMapp() = 0;
		uint64_t GetSize() const { return m_BufferSize; }
	protected:
		IGPUBuffer() : m_BufferSize(0), m_MemoryType(EMemoryType::E_MEMORY_TYPE_MAX), m_BufferUsages(0){}
		virtual ~IGPUBuffer() {}
		uint64_t					m_BufferSize;
		EMemoryType					m_MemoryType;
		std::vector<EBufferUsage>	m_BufferUsages;
	};
	using PGPUBuffer = IGPUBuffer*;

	class IGPUImage : public IObject
	{
	public:
		virtual void Dispose() override {}
		inline uint32_t GetImageWidth() const { return m_Width; }
		inline uint32_t GetImageHeight() const { return m_Height; }
		inline uint32_t GetImageLayerNum() const { return m_LayerNum; }
		void SetStaticLayout(bool static_layout) { m_StaticLayout = static_layout; }
		bool IsStaticLayout() const { return m_StaticLayout; }
	protected:
		IGPUImage() :
			m_StaticLayout(false),
			m_Format(EFormat::E_FORMAT_MAX),
			m_Width(0),
			m_Height(0),
			m_Depth(0),
			m_MipLevelNum(0),
			m_LayerNum(0),
			m_MemoryType(EMemoryType::E_MEMORY_TYPE_MAX),
			m_ImageUsages()
		{}
		virtual ~IGPUImage() {}
		bool						m_StaticLayout;
		EFormat						m_Format;
		uint32_t					m_Width;
		uint32_t					m_Height;
		uint32_t					m_Depth;
		uint32_t					m_MipLevelNum;
		uint32_t					m_LayerNum;
		EMemoryType					m_MemoryType;
		std::vector<EImageUsage>	m_ImageUsages;
	};
	using PGPUImage = IGPUImage*;

	class RenderPassAttachment
	{
	public:
		EFormat					m_Format;
		EAttachmentClearType	m_ClearType;
		uint32_t				m_SampleCount;
		RenderPassAttachment(EFormat format = EFormat::E_FORMAT_B8G8R8A8_SRGB, EAttachmentClearType clear_type = EAttachmentClearType::E_ATTACHMENT_NOT_CLEAR, uint32_t sample_count = 1) : 
			m_Format(format),
			m_ClearType(clear_type),
			m_SampleCount(sample_count)
		{}
	};

	class SubpassInfo
	{
	public:
		std::vector<int32_t>	m_OutputAttachments;
		std::vector<int32_t>	m_InputAttachments;
		std::vector<int32_t>	m_DepthInputAttachments;
		std::vector<int32_t>	m_StencilInputAttachments;
		std::vector<int32_t>	m_ResolveAttachments;
		int32_t					m_DepthStencilAttachment;
		bool					b_DepthStencilReadOnly;
		SubpassInfo() :
			m_DepthStencilAttachment(-1),
			b_DepthStencilReadOnly(false)
		{}
	};

	class RenderPass : IObject
	{
	public:

		std::vector<RenderPassAttachment>	m_Attachments;
		std::vector<SubpassInfo>			m_Subpasses;
		virtual void BuildRenderPass() = 0;
		virtual void InstanciatePipeline(GraphicsPipeline* pipeline, uint32_t subpass) = 0;
		virtual void Dispose() override = 0;
	protected:
		RenderPass() {}
		virtual ~RenderPass() {}
	};
	using PRenderPass = RenderPass*;

	class Framebuffer : public IObject
	{
	public:
		std::vector<PGPUImage>	m_Images;
		uint32_t				m_Width;
		uint32_t				m_Height;
		uint32_t				m_Layers;
	protected:
		Framebuffer() : 
			m_Width(0),
			m_Height(0),
			m_Layers(0)
		{}
		virtual ~Framebuffer() {}
	};
	using PFramebuffer = Framebuffer*;

	class RenderPassInstance : public IObject
	{
	public:
		virtual void Call() = 0;
	protected:
		PRenderPass		p_RenderPass;
		PFramebuffer	p_Framebuffer;
	};
	using PRenderPassInstance = RenderPassInstance*;
}