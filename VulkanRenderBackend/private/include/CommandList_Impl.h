#pragma once
#include <RenderInterface/header/CCommandList.h>
#include <private/include/VulkanIncludes.h>
namespace graphics_backend
{
	class CCommandList_Impl : public CInlineCommandList
	{
	public:
		CCommandList_Impl(vk::CommandBuffer cmd);
		virtual void BindVertexBuffers(std::vector<GPUBuffer const*> pGPUBuffers, std::vector<uint32_t> offsets) override;
		virtual void BindIndexBuffers(EIndexBufferType indexBufferType, GPUBuffer const* pGPUBuffer, uint32_t offset) override;
		virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1) override;
		virtual void Draw(uint32_t vertexCount, uint32_t instanceCount = 1) override;
	private:
		vk::CommandBuffer m_CommandBuffer = nullptr;
	};
}