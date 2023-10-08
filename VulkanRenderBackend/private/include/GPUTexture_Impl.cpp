#include "private/include/pch.h"
#include "GPUTexture_Impl.h"
#include "CVulkanApplication.h"
#include "VulkanBarrierCollector.h"
#include "InterfaceTranslator.h"

namespace graphics_backend
{
	GPUTexture_Impl::GPUTexture_Impl(CVulkanApplication& app) : BaseUploadingResource(app)
	{
	}
	void GPUTexture_Impl::ScheduleTextureData(uint64_t textureDataOffset, uint64_t dataSize, void* pData)
	{
		size_t scheduleSize = textureDataOffset + dataSize;
		if (scheduleSize > m_ScheduledData.size())
		{
			m_ScheduledData.resize(scheduleSize);
		}
		memcpy(m_ScheduledData.data() + textureDataOffset, pData, dataSize);
	}
	void GPUTexture_Impl::UploadAsync()
	{
		BaseUploadingResource::UploadAsync(UploadingResourceType::eMemoryDataLowPriority);
	}
	bool GPUTexture_Impl::UploadingDone() const
	{
		return BaseUploadingResource::UploadingDone();
	}
	void GPUTexture_Impl::Initialize(const GPUTextureDescriptor& descriptor)
	{
		m_Descriptor = descriptor;
	}
	void GPUTexture_Impl::Release()
	{
		m_Descriptor = {};
		m_ImageObject.RAIIRelease();
		m_ScheduledData.clear();
	}
	void GPUTexture_Impl::DoUpload()
	{
		auto& memoryManager = GetMemoryManager();
		auto threadContext = GetVulkanApplication().AquireThreadContextPtr();
		auto currentFrame = GetFrameCountContext().GetCurrentFrameID();

		std::atomic_thread_fence(std::memory_order_acquire);

		uint64_t byteSize = m_ScheduledData.size();
		auto tempBuffer = memoryManager.AllocateFrameBoundTransferStagingBuffer(byteSize);
		memcpy(tempBuffer->GetMappedPointer(), m_ScheduledData.data(), byteSize);

		m_ImageObject = memoryManager.AllocateImage(m_Descriptor
			, EMemoryType::GPU
			, EMemoryLifetime::Persistent);

		auto cmdBuffer = threadContext->GetCurrentFramePool().AllocateMiscCommandBuffer("Upload GPU Texture");

		VulkanBarrierCollector imageBarrier{ GetFrameCountContext().GetGraphicsQueueFamily() };
		imageBarrier.PushImageBarrier(m_ImageObject->GetImage()
			, ResourceUsage::eDontCare, ResourceUsage::eTransferDest);
		imageBarrier.ExecuteBarrier(cmdBuffer);

		std::array<vk::BufferImageCopy, 1> bufferImageCopy = { GPUTextureDescriptorToBufferImageCopy(m_Descriptor) };
		cmdBuffer.copyBufferToImage(tempBuffer->GetBuffer()
			, m_ImageObject->GetImage()
			, vk::ImageLayout::eTransferDstOptimal
			, bufferImageCopy);

		VulkanBarrierCollector imageBarrier1{ GetFrameCountContext().GetGraphicsQueueFamily() };
		imageBarrier1.PushImageBarrier(m_ImageObject->GetImage()
			, ResourceUsage::eTransferDest
			, ResourceUsageFlags{ ResourceUsage::eVertexRead }
			| ResourceUsage::eFragmentRead
			| ResourceUsage::eComputeRead);
		imageBarrier1.ExecuteBarrier(cmdBuffer);
		cmdBuffer.end();

		bool isDepthStencil = IsDepthStencilFormat(m_Descriptor.format);
		if (isDepthStencil)
		{
			m_DefaultImageView = GetVulkanApplication().CreateDefaultImageView(m_Descriptor, m_ImageObject->GetImage(), true, true);
		}
		else
		{
			m_DefaultImageView = GetVulkanApplication().CreateDefaultImageView(m_Descriptor, m_ImageObject->GetImage(), false, false);
		}

		m_DepthImageView = GetVulkanApplication().CreateDefaultImageView(m_Descriptor, m_ImageObject->GetImage(), true, false);
		m_StencilImageView = GetVulkanApplication().CreateDefaultImageView(m_Descriptor, m_ImageObject->GetImage(), false, true);

		MarkUploadingDoneThisFrame();
	}
}