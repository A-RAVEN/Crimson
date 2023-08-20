#include "private/include/pch.h"
#include "RenderGraphExecutor.h"
#include "CVulkanApplication.h"
#include "InterfaceTranslator.h"
#include "CommandList_Impl.h"
#include "VulkanPipelineObject.h"
#include "VulkanBarrierCollector.h"

namespace graphics_backend
{
	RenderGraphExecutor::RenderGraphExecutor(CVulkanApplication& owner) : BaseApplicationSubobject(owner)
	{
	}

	void RenderGraphExecutor::Create(std::shared_ptr<CRenderGraph> inRenderGraph)
	{
		m_RenderGraph = inRenderGraph;
	}
	void RenderGraphExecutor::Compile()
	{
		if (CompileIssued())
			return;
		auto creationTask = GetVulkanApplication().NewTask();
		creationTask->Name("Initialize RenderGraph");
		uint32_t nodeCount = m_RenderGraph->GetRenderNodeCount();
		creationTask->Functor([this, nodeCount]()
			{
				m_RenderPasses.clear();
				m_RenderPasses.reserve(nodeCount);
				for (uint32_t itr_node = 0; itr_node < nodeCount; ++itr_node)
				{
					auto& renderPass = m_RenderGraph->GetRenderPass(itr_node);
					m_RenderPasses.emplace_back(*this, *m_RenderGraph, renderPass);
				}
				
			});

		//处理每个RenderPass的Barrier
		auto resolvingTask = GetVulkanApplication().NewTask()
			->Name("Resolve Resource Usages")
			->Functor([this, nodeCount]()
				{
					m_TextureHandleUsageStates.clear();
					for (uint32_t itr_node = 0; itr_node < nodeCount; ++itr_node)
					{
						m_RenderPasses[itr_node].ResolveTextureHandleUsages(m_TextureHandleUsageStates);
					}
				});
		//编译每个RenderPass(FrameBuffer, RenderPass, PSO)
		for (uint32_t i = 0; i < nodeCount; ++i)
		{
			auto compileTask = GetVulkanApplication().NewTask();
			compileTask->Name("Compile RenderPass");
			compileTask->Succeed(creationTask);
			compileTask->Functor([this, i]()
				{
					m_RenderPasses[i].Compile();
				});
			resolvingTask->Succeed(compileTask);
		}
		m_CompiledFrame = GetVulkanApplication().GetSubmitCounterContext().GetCurrentFrameID();
	}

	void RenderGraphExecutor::Run()
	{
		Compile();
		Execute();
	}

	bool RenderGraphExecutor::CompileDone() const
	{
		if (m_CompiledFrame == INVALID_FRAMEID)
			return false;
		auto& frameContext = GetVulkanApplication().GetSubmitCounterContext();
		if (!frameContext.AnyFrameFinished())
			return false;
		return frameContext.GetReleasedFrameID() >= m_CompiledFrame;
	}

	bool RenderGraphExecutor::CompileIssued() const
	{
		return m_CompiledFrame != INVALID_FRAMEID;
	}

	void RenderGraphExecutor::CollectCommands(std::vector<vk::CommandBuffer>& inoutCommands)
	{
		inoutCommands.resize(inoutCommands.size() + m_PendingGraphicsCommandBuffers.size());
		std::copy(m_PendingGraphicsCommandBuffers.begin()
			, m_PendingGraphicsCommandBuffers.end()
			, inoutCommands.end() - m_PendingGraphicsCommandBuffers.size());
	}

	void RenderGraphExecutor::Execute()
	{
		if (!CompileDone())
			return;

		auto collectCommandsTask = GetVulkanApplication().NewTask()
			->Name("Collect RenderPass Commands");


		uint32_t nodeCount = m_RenderGraph->GetRenderNodeCount();
		for (uint32_t i = 0; i < nodeCount; ++i)
		{
			auto recorderTask = GetVulkanApplication().NewTask();
			recorderTask->Name("Record RenderPass Cmds");
			recorderTask->Functor([this, i]()
				{
					CVulkanThreadContext& threadContext = GetVulkanApplication().AquireThreadContext();
					m_RenderPasses[i].SetupFrameBuffer();
					m_RenderPasses[i].PrepareCommandBuffers(threadContext);
					GetVulkanApplication().ReturnThreadContext(threadContext);
				});
			collectCommandsTask->Succeed(recorderTask);
		}

		collectCommandsTask->Functor([this, nodeCount]()
			{
				m_PendingGraphicsCommandBuffers.clear();
				for (uint32_t i = 0; i < nodeCount; ++i)
				{
					m_RenderPasses[i].AppendCommandBuffers(m_PendingGraphicsCommandBuffers);
				}
				auto windowTarget = m_RenderGraph->GetTargetWindow<CWindowContext>();

				std::array<const vk::Semaphore, 1> semaphore = { windowTarget->GetWaitDoneSemaphore()};
				std::array<const vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eTransfer };
				std::array<const vk::Semaphore, 1> signalSemaphore = { windowTarget->GetPresentWaitingSemaphore() };

				//GetFrameCountContext().SubmitGraphics(m_PendingGraphicsCommandBuffers, semaphore, waitStages);
				TIndex windowIndex = m_RenderGraph->WindowHandleToTextureIndex(windowTarget);
				auto state = m_TextureHandleUsageStates.find(windowIndex);
				if (state != m_TextureHandleUsageStates.end())
				{
					windowTarget->MarkUsages(state->second);
				}
			});
	}


	RenderPassExecutor::RenderPassExecutor(RenderGraphExecutor& owningExecutor, CRenderGraph const& renderGraph, CRenderpassBuilder const& renderpassBuilder) :
		m_OwningExecutor(owningExecutor)
		, m_RenderGraph(renderGraph)
		, m_RenderpassBuilder(renderpassBuilder)
	{
	}
	void RenderPassExecutor::ResolveTextureHandleUsages(std::unordered_map<TIndex, ResourceUsage>& textureHandleUsageStates)
	{
		auto& handles = m_RenderpassBuilder.GetTextureHandles();
		for (auto handleID : handles)
		{
			if (handleID != INVALID_INDEX)
			{
				ResourceUsage srcUsage = ResourceUsage::eDontCare;
				auto found = textureHandleUsageStates.find(handleID);
				if (found != textureHandleUsageStates.end())
				{
					srcUsage = found->second;
				}
				ResourceUsage dstUsage = ResourceUsage::eColorAttachmentOutput;
				textureHandleUsageStates[handleID] = dstUsage;
				if (srcUsage != dstUsage)
				{
					m_UsageBarriers.push_back(std::make_tuple(handleID, srcUsage, dstUsage));
				}
			}
		}
	}
	void RenderPassExecutor::Compile()
	{
		CompileRenderPass();
		CompilePSOs();
	}

	void RenderPassExecutor::ExecuteSubpass_SimpleDraw(
		uint32_t subpassID
		, uint32_t width
		, uint32_t height
		, vk::CommandBuffer cmd)
	{
		GPUObjectManager& gpuObjectManager = m_OwningExecutor.GetGPUObjectManager();
		auto& renderPassInfo = m_RenderpassBuilder.GetRenderPassInfo();
		RenderPassDescriptor rpDesc{ renderPassInfo };
		auto pRenderPass = gpuObjectManager.GetRenderPassCache().GetOrCreate(rpDesc).lock();

		auto& subpassData = m_RenderpassBuilder.GetSubpassData_SimpleDrawcall(subpassID);
		auto vertModule = gpuObjectManager.GetShaderModuleCache().GetOrCreate({ subpassData.shaderSet.vert }).lock();
		auto fragModule = gpuObjectManager.GetShaderModuleCache().GetOrCreate({ subpassData.shaderSet.frag }).lock();

		CPipelineObjectDescriptor pipelineDesc{
			subpassData.pipelineStateObject
			, subpassData.vertexInputDescriptor
			, ShaderStateDescriptor{vertModule, fragModule}
			, pRenderPass
			, subpassID };

		auto pPipeline = m_GraphicsPipelineObjects[subpassID];


		cmd.setViewport(0
			, {
				vk::Viewport{0.0f, 0.0f
				, static_cast<float>(width)
				, static_cast<float>(height)
				, 0.0f, 1.0f}
			}
		);
		cmd.setScissor(0
			, {
				vk::Rect2D{
					{0, 0}
					, { width, height }
				}
			}
		);
		cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pPipeline->GetPipeline());
		CCommandList_Impl cmdListInterface{ cmd, pRenderPass, subpassID };
		subpassData.commandFunction(cmdListInterface);
	}

	void RenderPassExecutor::ProcessAquireBarriers(vk::CommandBuffer cmd)
	{
		VulkanBarrierCollector textureBarrier{ m_OwningExecutor.GetFrameCountContext().GetGraphicsQueueFamily() };
		for (auto& usageData : m_UsageBarriers)
		{
			auto& textureInfo = m_RenderGraph.GetTextureHandleInternalInfo(std::get<0>(usageData));
			textureBarrier.PushImageBarrier(static_cast<CWindowContext*>(textureInfo.p_WindowsHandle.get())->GetCurrentFrameImage()
				, std::get<1>(usageData)
				, std::get<2>(usageData));
		}
		textureBarrier.ExecuteBarrier(cmd);
	}

	void RenderPassExecutor::PrepareCommandBuffers(CVulkanThreadContext& threadContext)
	{
		m_PendingGraphicsCommandBuffers.clear();

		vk::CommandBuffer cmd = threadContext.GetCurrentFramePool().AllocateOnetimeCommandBuffer("Render Pass Command Buffer");

		ProcessAquireBarriers(cmd);

		auto& renderpassInfo = m_RenderpassBuilder.GetRenderPassInfo();

		cmd.beginRenderPass(
			vk::RenderPassBeginInfo{
			m_RenderPassObject->GetRenderPass()
				, m_FrameBufferObject->GetFramebuffer()
				, vk::Rect2D{{0, 0}
			, { m_FrameBufferObject->GetWidth()
				, m_FrameBufferObject->GetHeight() }}
			, m_ClearValues}
		, vk::SubpassContents::eInline);

		for (uint32_t subpassId = 0; subpassId < renderpassInfo.subpassInfos.size(); ++subpassId)
		{
			ESubpassType subpasType = m_RenderpassBuilder.GetSubpassType(subpassId);
			if (subpassId > 0)
			{
				cmd.nextSubpass(vk::SubpassContents::eInline);
			}
			switch (subpasType)
			{
			case ESubpassType::eSimpleDraw:
				ExecuteSubpass_SimpleDraw(
					subpassId
					, m_FrameBufferObject->GetWidth()
					, m_FrameBufferObject->GetHeight()
					, cmd);
				break;
			case ESubpassType::eMultiDrawInterface:
				break;
			}
		}
		cmd.endRenderPass();
		cmd.end();
		m_PendingGraphicsCommandBuffers.push_back(cmd);
	}
	void RenderPassExecutor::AppendCommandBuffers(std::vector<vk::CommandBuffer>& outCommandBuffers)
	{
		outCommandBuffers.resize(outCommandBuffers.size() + m_PendingGraphicsCommandBuffers.size());
		std::copy(m_PendingGraphicsCommandBuffers.begin(), m_PendingGraphicsCommandBuffers.end(), outCommandBuffers.end() - m_PendingGraphicsCommandBuffers.size());
	}
	void RenderPassExecutor::SetupFrameBuffer()
	{
		GPUObjectManager& gpuObjectManager = m_OwningExecutor.GetGPUObjectManager();
		auto& handles = m_RenderpassBuilder.GetTextureHandles();
		m_FrameBufferImageViews.clear();
		m_FrameBufferImageViews.reserve(handles.size());

		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t layers = 0;
		bool firstHandle = true;
		for (TIndex handleIDS : handles)
		{
			TextureHandleInternalInfo const& textureInfo = m_RenderGraph.GetTextureHandleInternalInfo(handleIDS);
			TextureHandle textureHandle = m_RenderGraph.TextureHandleByIndex(handleIDS);
			GPUTextureDescriptor const& desc = textureHandle.GetDescriptor();

			if (firstHandle)
			{
				firstHandle = false;
				width = desc.width;
				height = desc.height;
				layers = desc.layers;
			}
			else
			{
				CA_ASSERT((width == desc.width && height == desc.height && layers == desc.layers)
					, "RenderPassExecutor::Compile() : All texture handles must have same width, height and layers");
			}

			if (textureInfo.p_WindowsHandle != nullptr)
			{
				m_FrameBufferImageViews.push_back(static_cast<CWindowContext*>(textureInfo.p_WindowsHandle.get())->GetCurrentFrameImageView());
			}
		}

		FramebufferDescriptor framebufferDesc{
			m_FrameBufferImageViews
			, m_RenderPassObject
			, width, height, layers
		};
		m_FrameBufferObject = gpuObjectManager.GetFramebufferCache().GetOrCreate(framebufferDesc).lock();
	}
	void RenderPassExecutor::CompileRenderPass()
	{
		auto& renderpassInfo = m_RenderpassBuilder.GetRenderPassInfo();
		RenderPassDescriptor rpDesc{ renderpassInfo };
		GPUObjectManager& gpuObjectManager = m_OwningExecutor.GetGPUObjectManager();
		m_RenderPassObject = gpuObjectManager.GetRenderPassCache().GetOrCreate(rpDesc).lock();

		m_ClearValues.resize(renderpassInfo.attachmentInfos.size());
		for (uint32_t attachmentID = 0; attachmentID < renderpassInfo.attachmentInfos.size(); ++attachmentID)
		{
			auto& attachmentInfo = renderpassInfo.attachmentInfos[attachmentID];
			m_ClearValues[attachmentID] = AttachmentClearValueTranslate(
				attachmentInfo.clearValue
				, attachmentInfo.format);
		}


	}
	void RenderPassExecutor::CompilePSOs()
	{
		GPUObjectManager& gpuObjectManager = m_OwningExecutor.GetGPUObjectManager();
		auto& renderpassInfo = m_RenderpassBuilder.GetRenderPassInfo();
		uint32_t subpassCount = renderpassInfo.subpassInfos.size();
		m_GraphicsPipelineObjects.resize(subpassCount);
		for (uint32_t subpassID = 0; subpassID < subpassCount; ++subpassID)
		{
			auto& subpassData = m_RenderpassBuilder.GetSubpassData_SimpleDrawcall(subpassID);
			auto vertModule = gpuObjectManager.GetShaderModuleCache().GetOrCreate({ subpassData.shaderSet.vert }).lock();
			auto fragModule = gpuObjectManager.GetShaderModuleCache().GetOrCreate({ subpassData.shaderSet.frag }).lock();

			CPipelineObjectDescriptor pipelineDesc{
			subpassData.pipelineStateObject
			, subpassData.vertexInputDescriptor
			, ShaderStateDescriptor{vertModule, fragModule}
			, m_RenderPassObject
			, subpassID };

			m_GraphicsPipelineObjects[subpassID] = gpuObjectManager.GetPipelineCache().GetOrCreate(pipelineDesc).lock();
		}
	}
}
