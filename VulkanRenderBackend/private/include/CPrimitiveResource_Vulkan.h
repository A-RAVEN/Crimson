#pragma once
#include <RenderInterface/header/CGPUPrimitiveResource.h>
#include "CVulkanBufferObject.h"
#include "CVulkanThreadContext.h"

namespace graphics_backend
{
	class CGPUPrimitiveResource_Vulkan : public CGPUPrimitiveResource
	{
	public:
		CGPUPrimitiveResource_Vulkan(CVulkanApplication* owningApplication);
		// 通过 CGPUPrimitiveResource 继承
		virtual void AddPrimitiveDescriptor(uint32_t stride, std::vector<VertexAttribute> const& attributes, bool perInstance) override;
		virtual void SetPrimitiveData(uint32_t primitiveIndex, uint32_t dataSize, void* data) override;
		virtual void SetIndexData(uint32_t dataSize, void* data, bool bit16Index) override;
		virtual void Submit() override;
		virtual bool GPUDone() override;
		void Release();
		void Initialize(){}
	private:
		CVulkanApplication* p_OwningApplication;

		std::shared_ptr<CVulkanBufferObject> m_PrimitiveDataBuffer;
		std::shared_ptr<CVulkanBufferObject> m_PrimitiveIndexDataBuffer;
		bool m_16BitIndices = false;
		std::vector<uint8_t> m_IndicesDataCache;
		std::vector<std::vector<uint8_t>> m_PrimitiveDataCache;
		std::vector<std::tuple<uint32_t, std::vector<VertexAttribute>, bool>> m_PrimitiveDescriptions;
		FrameType m_SubmitFrame = INVALID_FRAMEID;
	};
}