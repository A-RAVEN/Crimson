#include "private/include/pch.h"
#include "private/include/CPrimitiveResource_Vulkan.h"
#include <private/include/CVulkanApplication.h>
#include <private/include/CVulkanThreadContext.h>

namespace graphics_backend
{
	CGPUPrimitiveResource_Vulkan::CGPUPrimitiveResource_Vulkan(CVulkanApplication* owningApplication) :
		p_OwningApplication(owningApplication)
	{
	}

    //void CGPUPrimitiveResource_Vulkan::AddPrimitiveDescriptor(uint32_t stride, std::vector<VertexAttribute> const& attributes, bool perInstance)
    //{
    //    m_PrimitiveDescriptions.push_back(std::make_tuple(stride, attributes, perInstance));
    //}

    void CGPUPrimitiveResource_Vulkan::SetPrimitiveData(uint32_t primitiveIndex, uint32_t dataSize, void* data)
    {
        if (primitiveIndex >= m_PrimitiveDataCache.size())
        {
            m_PrimitiveDataCache.resize(primitiveIndex + 1);
        }
        std::vector<uint8_t>& refByteVec = m_PrimitiveDataCache[primitiveIndex];
        refByteVec.resize(dataSize);
        memcpy(refByteVec.data(), data, dataSize);
    }

    void CGPUPrimitiveResource_Vulkan::SetIndexData(uint32_t dataSize, void* data, bool bit16Index)
    {
        m_16BitIndices = bit16Index;
        uint32_t sizeInBytes = dataSize * (bit16Index ? 2 : 4);
        m_IndicesDataCache.resize(sizeInBytes);
        memcpy(m_IndicesDataCache.data(), data, sizeInBytes);
    }

    void CGPUPrimitiveResource_Vulkan::Submit()
    {
        if (m_VertexInputDescriptor.m_PrimitiveDescriptions.size() == 0)
            return;
        if (m_PrimitiveDataCache.size() == 0)
            return;
        assert(m_PrimitiveDataCache.size() == m_VertexInputDescriptor.m_PrimitiveDescriptions.size());
#if !defined(NDEBUG)
        {
            for (uint32_t i = 0; i < m_PrimitiveDataCache.size(); ++i)
            {
                size_t stride = std::get<0>(m_VertexInputDescriptor.m_PrimitiveDescriptions[i]);
                assert(((m_PrimitiveDataCache[i].size()) / stride * stride) == m_PrimitiveDataCache[i].size());
            }
        }
#endif

        auto task = p_OwningApplication->NewTask()->Name("Upload Primitive Task");
        task->Functor([this]()
            {
                auto& memoryManager = p_OwningApplication->GetMemoryManager();
                auto& threadContext = p_OwningApplication->AquireThreadContext();
                auto currentFrame = p_OwningApplication->GetSubmitCounterContext().GetCurrentFrameID();

                std::atomic_thread_fence(std::memory_order_acquire);
                size_t vertexDataSize = 0;
                for (uint32_t i = 0; i < m_PrimitiveDataCache.size(); ++i)
                {
                    vertexDataSize += m_PrimitiveDataCache[i].size();
                }
                std::vector<uint8_t> byteArray;
                byteArray.reserve(vertexDataSize);
                for (uint32_t i = 0; i < m_PrimitiveDataCache.size(); ++i)
                {
                    byteArray.insert(byteArray.end(), m_PrimitiveDataCache[i].begin(), m_PrimitiveDataCache[i].end());
                }
                m_PrimitiveDataBuffer = memoryManager.AllocateBuffer(EMemoryType::GPU
                    , EMemoryLifetime::Persistent
                    , byteArray.size(), vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst);
                auto tempBuffer = memoryManager.AllocateBuffer(EMemoryType::CPU_Sequential_Access, EMemoryLifetime::FrameBound, byteArray.size(), vk::BufferUsageFlagBits::eTransferSrc);
                memcpy(tempBuffer.GetMappedPointer(), byteArray.data(), byteArray.size());
                auto cmdBuffer = threadContext.GetCurrentFramePool().AllocateOnetimeCommandBuffer();
                cmdBuffer.copyBuffer(tempBuffer.GetBuffer(), m_PrimitiveDataBuffer.GetBuffer(), vk::BufferCopy(0, 0, byteArray.size()));
                cmdBuffer.end();
                tempBuffer.Release();
                std::atomic_thread_fence(std::memory_order_release);
                m_SubmitFrame = currentFrame;
                p_OwningApplication->ReturnThreadContext(threadContext);
            });
    }

    bool CGPUPrimitiveResource_Vulkan::GPUDone()
    {
        if (m_SubmitFrame == INVALID_FRAMEID)
            return false;
        auto& frameContext = p_OwningApplication->GetSubmitCounterContext();
        if (!frameContext.AnyFrameFinished())
            return false;
        return frameContext.GetReleasedFrameID() >= m_SubmitFrame;
    }

    void CGPUPrimitiveResource_Vulkan::Release()
    {
        m_PrimitiveDataBuffer.Release();
        m_PrimitiveIndexDataBuffer.Release();
        m_PrimitiveDataBuffer = CVulkanBufferObject{};
        m_PrimitiveIndexDataBuffer = CVulkanBufferObject{};
		m_16BitIndices = false;
        m_IndicesDataCache.clear();
        m_PrimitiveDataCache.clear();
        m_VertexInputDescriptor.m_PrimitiveDescriptions.clear();
		m_SubmitFrame = INVALID_FRAMEID;
    }
}

