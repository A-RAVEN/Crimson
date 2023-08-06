#include <private/include/pch.h>
#include <private/include/CommandList_Impl.h>
#include <private/include/GPUBuffer_Impl.h>
#include <private/include/InterfaceTranslator.h>

namespace graphics_backend
{
	CCommandList_Impl::CCommandList_Impl(vk::CommandBuffer cmd) : m_CommandBuffer(cmd)
	{
	}
	void CCommandList_Impl::BindVertexBuffers(std::vector<GPUBuffer const*> pGPUBuffers, std::vector<uint32_t> offsets)
	{
		std::vector<vk::Buffer> gpuBufferList;
		std::vector<vk::DeviceSize> offsetList;
		gpuBufferList.resize(pGPUBuffers.size());
		offsetList.resize(pGPUBuffers.size());
		for (uint32_t i = 0; i < pGPUBuffers.size(); ++i)
		{
			gpuBufferList[i] = static_cast<GPUBuffer_Impl const*>(pGPUBuffers[i])
				->GetVulkanBufferObject().GetBuffer();
		}
		std::fill(offsetList.begin(), offsetList.end(), 0);
		for (uint32_t i = 0; i < offsets.size(); ++i)
		{
			offsetList[i] = offsets[i];
		}
		m_CommandBuffer.bindVertexBuffers(0, gpuBufferList, offsetList);
	}
	void CCommandList_Impl::BindIndexBuffers(EIndexBufferType indexBufferType, GPUBuffer const* pGPUBuffer, uint32_t offset)
	{
		m_CommandBuffer.bindIndexBuffer(static_cast<GPUBuffer_Impl const*>(pGPUBuffer)
			->GetVulkanBufferObject().GetBuffer(), offset, EIndexBufferTypeTranslate(indexBufferType));
	}
	void CCommandList_Impl::DrawIndexed(uint32_t indexCount, uint32_t instanceCount)
	{
		m_CommandBuffer.drawIndexed(indexCount, instanceCount, 0, 0, 0);
	}
	void CCommandList_Impl::Draw(uint32_t vertexCount, uint32_t instanceCount)
	{
		m_CommandBuffer.draw(vertexCount, instanceCount, 0, 0);
	}
}