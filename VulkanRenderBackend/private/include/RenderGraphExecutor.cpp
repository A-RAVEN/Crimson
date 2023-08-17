#include "private/include/pch.h"
#include "RenderGraphExecutor.h"
#include "CVulkanApplication.h"
#include "InterfaceTranslator.h"
#include "CommandList_Impl.h"
#include "VulkanPipelineObject.h"

namespace graphics_backend
{
	RenderGraphExecutor::RenderGraphExecutor(CVulkanApplication& owner) : BaseApplicationSubobject(owner)
	{
	}

	void RenderGraphExecutor::Create(std::shared_ptr<CRenderGraph> inRenderGraph)
	{
		m_RenderGraph = m_RenderGraph;
	}
	void RenderGraphExecutor::Compile()
	{
		if (CompileDone())
			return;
		auto task = GetVulkanApplication().NewTask();
		task->Name("Compile RenderGraph");
		uint32_t nodeCount = m_RenderGraph->GetRenderNodeCount();
		task->Functor([this, nodeCount]()
			{
				m_RenderPasses.clear();
				m_RenderPasses.reserve(nodeCount);
				for (uint32_t itr_node = 0; itr_node < nodeCount; ++itr_node)
				{
					auto& renderPass = m_RenderGraph->GetRenderPass(itr_node);
					m_RenderPasses.emplace_back(*this, *m_RenderGraph, renderPass);
				}
				
			});


		for (uint32_t i = 0; i < nodeCount; ++i)
		{
			auto compileTask = GetVulkanApplication().NewTask();
			compileTask->Name("Compile RenderPass");
			compileTask->Succeed(task);
			compileTask->Functor([this, i]()
				{
					m_RenderPasses[i].Compile();
				});
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

	void RenderGraphExecutor::Execute()
	{
		if (!CompileDone())
			return;
		uint32_t nodeCount = m_RenderGraph->GetRenderNodeCount();
		for (uint32_t i = 0; i < nodeCount; ++i)
		{
			auto recorderTask = GetVulkanApplication().NewTask();
			recorderTask->Name("Record RenderPass Cmds");
			recorderTask->Functor([this, i]()
				{
					CVulkanThreadContext& threadContext = GetVulkanApplication().AquireThreadContext();
					vk::CommandBuffer cmd = threadContext.GetCurrentFramePool().AllocateOnetimeCommandBuffer();
					m_RenderPasses[i].Execute(cmd);
					cmd.end();
				});
		}
	}


	RenderPassExecutor::RenderPassExecutor(RenderGraphExecutor& owningExecutor, CRenderGraph const& renderGraph, CRenderpassBuilder const& renderpassBuilder) :
		m_OwningExecutor(owningExecutor)
		, m_RenderGraph(renderGraph)
		, m_RenderpassBuilder(renderpassBuilder)
	{
	}
	void RenderPassExecutor::Compile()
	{
		CompileRenderPassAndFrameBuffer();
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

	void RenderPassExecutor::Execute(vk::CommandBuffer cmd)
	{
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
	}
	void RenderPassExecutor::CompileRenderPassAndFrameBuffer()
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

		auto& handles = m_RenderpassBuilder.GetTextureHandles();
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
