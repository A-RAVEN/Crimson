#pragma once
#include <CGPUPrimitiveResource.h>
#include "CVulkanBufferObject.h"
#include "CVulkanThreadContext.h"

namespace graphics_backend
{
	class CGPUPrimitiveResource_Vulkan : CGPUPrimitiveResource
	{
	public:
		// 通过 CGPUPrimitiveResource 继承
		virtual void AddPrimitiveDescriptor(uint32_t stride, std::vector<VertexAttribute> const& attributes, bool perInstance) override;
		virtual void SetPrimitiveData(uint32_t primitiveIndex, uint32_t dataSize, void* data) override;
		virtual void SetIndexData(uint32_t dataSize, void* data, bool bit16Index) override;
		virtual bool GPUDone() override;

		void GPUPrepareResource(CVulkanThreadContext& threadContext);
	private:
		CVulkanBufferObject m_PrimitiveDataBuffer_PerVertex;
		CVulkanBufferObject m_PrimitiveDataBuffer_PerInstance;
		CVulkanBufferObject m_PrimitiveIndexDataBuffer;
	};
}