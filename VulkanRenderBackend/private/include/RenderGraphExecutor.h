#pragma once
#include "VulkanApplicationSubobjectBase.h"
#include <ThreadManager/header/ThreadManager.h>
#include <RenderInterface/header/CRenderGraph.h>
#include <memory>
#include "RenderPassObject.h"
#include "FramebufferObject.h"
#include "VulkanPipelineObject.h"
#include "RenderBackendSettings.h"

namespace graphics_backend
{
	class RenderGraphExecutor;

	//RenderPass Executor
	class RenderPassExecutor
	{
	public:
		RenderPassExecutor(RenderGraphExecutor& owningExecutor, CRenderGraph const& renderGraph, CRenderpassBuilder const& renderpassBuilder);
		void Compile();
		void Execute(vk::CommandBuffer cmd);
	private:
		void CompileRenderPassAndFrameBuffer();
		void CompilePSOs();

		void ExecuteSubpass_SimpleDraw(
			uint32_t subpassID
			, uint32_t width
			, uint32_t height
			, vk::CommandBuffer cmd);

		RenderGraphExecutor& m_OwningExecutor;
		CRenderpassBuilder const& m_RenderpassBuilder;
		CRenderGraph const& m_RenderGraph;

		//RenderPass and Framebuffer
		std::vector<vk::ImageView> m_FrameBufferImageViews;
		std::shared_ptr<RenderPassObject> m_RenderPassObject;
		std::shared_ptr<FramebufferObject> m_FrameBufferObject;
		std::vector<vk::ClearValue> m_ClearValues;

		//Pipeline States
		std::vector<std::shared_ptr<CPipelineObject>> m_GraphicsPipelineObjects;

	};

	//RenderGraph Executor
	class RenderGraphExecutor : public BaseApplicationSubobject
	{
	public:
		RenderGraphExecutor(CVulkanApplication& owner);
		void Create(std::shared_ptr<CRenderGraph> inRenderGraph);
		void Run();
		bool CompileDone() const;
	private:
		void Compile();
		void Execute();
		bool m_Compiled = false;

		std::shared_ptr<CRenderGraph> m_RenderGraph = nullptr;
		std::vector<RenderPassExecutor> m_RenderPasses;

		FrameType m_CompiledFrame = INVALID_FRAMEID;
	};

	using RenderGraphExecutorDic = HashPool<std::shared_ptr<CRenderGraph>, RenderGraphExecutor>;
}
