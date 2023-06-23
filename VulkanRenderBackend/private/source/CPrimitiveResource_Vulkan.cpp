#include "private/include/pch.h"
#include "private/include/CPrimitiveResource_Vulkan.h"
#include <private/include/CVulkanApplication.h>
#include <private/include/CVulkanThreadContext.h>

namespace graphics_backend
{
    void CGPUPrimitiveResource_Vulkan::AddPrimitiveDescriptor(uint32_t stride, std::vector<VertexAttribute> const& attributes, bool perInstance)
    {
    }

    void CGPUPrimitiveResource_Vulkan::SetPrimitiveData(uint32_t primitiveIndex, uint32_t dataSize, void* data)
    {
    }

    void CGPUPrimitiveResource_Vulkan::SetIndexData(uint32_t dataSize, void* data, bool bit16Index)
    {
    }

    bool CGPUPrimitiveResource_Vulkan::GPUDone()
    {
        return false;
    }
    void CGPUPrimitiveResource_Vulkan::GPUPrepareResource(CVulkanThreadContext& threadContext)
    {
        CVulkanApplication const* pApp = threadContext.GetVulkanApplication();
        auto device = pApp->GetDevice();

        vk::BufferCreateInfo bufferCreateInfo({}
            , 1u
            , vk::BufferUsageFlagBits::eVertexBuffer
            , vk::SharingMode::eExclusive);
        vk::Buffer vertexBuffer = device.createBuffer(bufferCreateInfo);

        CVulkanFrameBoundCommandBufferPool& currentFrameCmdPool = threadContext.GetCurrentFramePool();
        vk::CommandBuffer cmdBuffer = currentFrameCmdPool.AllocateCommandBuffer();
        cmdBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
        //cmdBuffer.copyBuffer()
        cmdBuffer.end();
    }
}

