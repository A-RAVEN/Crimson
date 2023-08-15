#include "private/include/pch.h"
#include "RenderGraphExecutor.h"
#include "CVulkanApplication.h"

namespace graphics_backend
{
	void RenderGraphExecutor::Create(std::shared_ptr<CRenderGraph> inRenderGraph)
	{
		m_RenderGraph = m_RenderGraph;
	}
	void RenderGraphExecutor::Compile()
	{
		auto task = GetVulkanApplication().NewTask();
		task->Name("Compile RenderGraph");
		task->Functor([this]()
			{
				uint32_t nodeCount = m_RenderGraph->GetRenderNodeCount();
				m_RenderPasses.clear();
				m_RenderPasses.reserve(nodeCount);
				for (uint32_t itr_node = 0; itr_node < nodeCount; ++itr_node)
				{
					auto& renderPass = m_RenderGraph->GetRenderPass(itr_node);
					m_RenderPasses.emplace_back(*m_RenderGraph, renderPass);
					m_RenderPasses.back().Compile();
				}
			});
	}
	RenderPassExecutor::RenderPassExecutor(RenderGraphExecutor& owningExecutor, CRenderGraph const& renderGraph, CRenderpassBuilder const& renderpassBuilder) :
		m_OwningExecutor(owningExecutor)
		, m_RenderGraph(renderGraph)
		, m_RenderpassBuilder(renderpassBuilder)
	{
	}
	void RenderPassExecutor::Compile()
	{
		auto& renderpassInfo = m_RenderpassBuilder.GetRenderPassInfo();

		RenderPassDescriptor rpDesc{ renderpassInfo };
		GPUObjectManager& gpuObjectManager = m_OwningExecutor.GetGPUObjectManager();
		m_RenderPassObject = gpuObjectManager.GetRenderPassCache().GetOrCreate(rpDesc).lock();

		auto& handles = m_RenderpassBuilder.GetTextureHandles();
		m_FrameBufferImageViews.reserve(handles.size());
		for (TIndex handleIDS : handles)
		{
			TextureHandleInternalInfo const& textureInfo = m_RenderGraph.GetTextureHandleInternalInfo(handleIDS);
			if (textureInfo.p_WindowsHandle != nullptr)
			{
				m_FrameBufferImageViews.push_back(static_cast<CWindowContext*>(textureInfo.p_WindowsHandle.get())->GetCurrentFrameImageView());
			}
		}
	}
}
