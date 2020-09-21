#pragma once
#include <Generals.h>
#include <Pipeline.h>
#include <GPUDevice.h>
#include <deque>
#include <headers/Debug.h>
#include <headers/LuaInterface/LuaInterfaces.h>

using namespace Crimson;

class GraphicsContext
{
public:
	GraphicsContext(PGPUDevice device);
	PGPUDevice m_Device;
	std::vector<PGPUImage> m_RenderTargets;
	std::vector<PFramebuffer> m_FrameBuffers;
	std::vector<PGraphicsPipeline> m_LocalGraphicsPipelines;
	std::vector<PRenderPass> m_RenderPasses;
	std::vector<PRenderPassInstance> m_RenderPassInstances;

	Ptr64 CreateResizableRenderTarget(uint32_t format, bool can_sample, bool can_blit);
	Ptr64 CreateFramebuffer(std::vector<Ptr64> const& renderTargets);
	Ptr64 CreateRenderPassInstance(Ptr64 renderPass, Ptr64 frameBuffer);
	Ptr64 AddRenderPass(PRenderPass renderPass);
	Ptr64 AddLocalGraphicsPipelines(PGraphicsPipeline pipeline);
	void ClearContext();
};