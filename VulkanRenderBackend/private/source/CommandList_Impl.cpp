#include <private/include/CommandList_Impl.h>

namespace graphics_backend
{
	CCommandList_Impl::CCommandList_Impl(vk::CommandBuffer cmd) : m_CommandBuffer(cmd)
	{
	}
	void CCommandList_Impl::BindVertexBuffers(std::vector<GPUBuffer const*> pGPUBuffers, std::vector<uint32_t> offsets)
	{

		m_CommandBuffer.bindVertexBuffers(0, )
	}
}