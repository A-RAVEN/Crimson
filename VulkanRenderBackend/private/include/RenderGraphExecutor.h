#pragma once
#include "VulkanApplicationSubobjectBase.h"
#include <ThreadManager/header/ThreadManager.h>
#include <RenderInterface/header/CRenderGraph.h>
#include <memory>
#include "RenderPassObject.h"
#include "FramebufferObject.h"
#include "VulkanPipelineObject.h"
#include "RenderBackendSettings.h"
#include "ResourceUsageInfo.h"

namespace graphics_backend
{
	class RenderGraphExecutor;

	//RenderPass Executor
	class RenderPassExecutor
	{
	public:
		RenderPassExecutor(RenderGraphExecutor& owningExecutor, CRenderGraph const& renderGraph, CRenderpassBuilder const& renderpassBuilder);
		void Compile();

		void ResolveTextureHandleUsages(std::unordered_map<TIndex, ResourceUsage>& TextureHandleUsageStates);

		void PrepareCommandBuffers(CVulkanThreadContext& threadContext);
		void AppendCommandBuffers(std::vector<vk::CommandBuffer>& outCommandBuffers);
		void SetupFrameBuffer();
	private:
		void CompileRenderPass();
		void CompilePSOs();

		void ProcessAquireBarriers(vk::CommandBuffer cmd);

		void ExecuteSubpass_SimpleDraw(
			uint32_t subpassID
			, uint32_t width
			, uint32_t height
			, vk::CommandBuffer cmd);

		RenderGraphExecutor& m_OwningExecutor;
		CRenderpassBuilder const& m_RenderpassBuilder;
		CRenderGraph const& m_RenderGraph;

		//RenderPass and Framebuffer
		std::shared_ptr<RenderPassObject> m_RenderPassObject;
		std::vector<vk::ClearValue> m_ClearValues;

		//Framebuffer
		std::vector<vk::ImageView> m_FrameBufferImageViews;
		std::shared_ptr<FramebufferObject> m_FrameBufferObject;

		//Pipeline States
		std::vector<std::shared_ptr<CPipelineObject>> m_GraphicsPipelineObjects;

		//TextureUsages
		std::vector<std::tuple<TIndex, ResourceUsage, ResourceUsage>> m_UsageBarriers;

		//CommandBuffers
		std::vector<vk::CommandBuffer> m_PendingGraphicsCommandBuffers;
	};

	class PresentExecutor
	{

	};

	//RenderGraph Executor
	class RenderGraphExecutor : public BaseApplicationSubobject
	{
	public:
		RenderGraphExecutor(CVulkanApplication& owner);
		void Create(std::shared_ptr<CRenderGraph> inRenderGraph);
		void Run(thread_management::CTaskGraph* taskGrap);
		bool CompileDone() const;
		bool CompileIssued() const;
		void CollectCommands(std::vector<vk::CommandBuffer>& inoutCommands);
	private:
		void Compile(thread_management::CTaskGraph* taskGrap);
		void Execute(thread_management::CTaskGraph* taskGrap);
		bool m_Compiled = false;

		std::shared_ptr<CRenderGraph> m_RenderGraph = nullptr;
		std::vector<RenderPassExecutor> m_RenderPasses;

		std::unordered_map<TIndex, ResourceUsage> m_TextureHandleUsageStates;
		std::vector<vk::CommandBuffer> m_PendingGraphicsCommandBuffers;

		FrameType m_CompiledFrame = INVALID_FRAMEID;
	};

	using RenderGraphExecutorDic = HashPool<std::shared_ptr<CRenderGraph>, RenderGraphExecutor>;
}
