#include "private/include/pch.h"
#include "private/include/CPrimitiveResource_Vulkan.h"
#include <private/include/CVulkanApplication.h>
#include <private/include/CVulkanThreadContext.h>

namespace graphics_backend
{
    void CGPUPrimitiveResource_Vulkan::AddPrimitiveDescriptor(uint32_t stride, std::vector<VertexAttribute> const& attributes, bool perInstance)
    {
        m_PrimitiveDescriptions.push_back(std::make_tuple(stride, attributes, perInstance));
    }

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
        if (m_PrimitiveDescriptions.size() == 0)
            return;
        if (m_PrimitiveDataCache.size() == 0)
            return;
        assert(m_PrimitiveDataCache.size() == m_PrimitiveDescriptions.size());
#if !defined(NDEBUG)
        {
            for (uint32_t i = 0; i < m_PrimitiveDataCache.size(); ++i)
            {
                size_t stride = std::get<0>(m_PrimitiveDescriptions[i]);
                assert(((m_PrimitiveDataCache[i].size()) / stride * stride) == m_PrimitiveDataCache[i].size());
            }
        }
#endif
        auto& threadContext = p_OwningApplication->AquireThreadContext();
        CThreadManager* threadManager = p_OwningApplication->GetThreadManager();
        threadManager->EnqueueAnyThreadWorkWithPromise([this, &threadContext, threadManager]()
            {
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
                m_PrimitiveDataBuffer = threadContext.AllocBufferObject(true, byteArray.size(), vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst);
                auto tempBuffer = threadContext.AllocBufferObject(false, byteArray.size(), vk::BufferUsageFlagBits::eTransferSrc);
                memcpy(tempBuffer->GetMappedPointer(), byteArray.data(), byteArray.size());
                auto cmdBuffer = threadContext.GetCurrentFramePool().AllocateOnetimeCommandBuffer();
                cmdBuffer.copyBuffer(tempBuffer->GetBuffer(), m_PrimitiveDataBuffer->GetBuffer(), vk::BufferCopy(0, 0, m_PrimitiveDataBuffer->GetAllocationInfo().size));
                cmdBuffer.end();
                std::atomic_thread_fence(std::memory_order_release);
                m_Loaded = true;
                p_OwningApplication->ReturnThreadContext(threadContext);
            });
    }

    bool CGPUPrimitiveResource_Vulkan::GPUDone()
    {
        return false;
    }
}

