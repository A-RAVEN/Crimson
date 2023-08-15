#pragma once
#include "VulkanApplicationSubobjectBase.h"
#include <ThreadManager/header/ThreadManager.h>
#include <RenderInterface/header/CRenderGraph.h>
#include <memory>

namespace graphics_backend
{
	class RenderGraphExecutor;

	class RenderPassExecutor
	{
	public:
		RenderPassExecutor(RenderGraphExecutor& owningExecutor, CRenderGraph const& renderGraph, CRenderpassBuilder const& renderpassBuilder);
		void Compile();
	private:
		RenderGraphExecutor& m_OwningExecutor;
		CRenderpassBuilder const& m_RenderpassBuilder;
		CRenderGraph const& m_RenderGraph;
		std::vector<vk::ImageView> m_FrameBufferImageViews;
		std::shared_ptr<RenderPassObject> m_RenderPassObject;
	};

	class RenderGraphExecutor : public BaseApplicationSubobject
	{
	public:
		RenderGraphExecutor(CVulkanApplication& owner);
		void Create(std::shared_ptr<CRenderGraph> inRenderGraph);
		void ScheduleExecute();
	private:
		void Compile();
		void Execute();
		bool m_Compiled = false;

		std::shared_ptr<CRenderGraph> m_RenderGraph = nullptr;
		std::vector<RenderPassExecutor> m_RenderPasses;
	};


}
