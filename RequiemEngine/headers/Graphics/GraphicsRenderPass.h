#pragma once
#include <Generals.h>
#include <Pipeline.h>
#include <GPUDevice.h>
#include <deque>
#include <headers/Debug.h>
#include <headers/LuaInterface/LuaInterfaces.h>

using namespace Crimson;

class GraphicsRenderPassInstance
{
public:
	PFramebuffer m_FrameBuffer;
	PRenderPassInstance m_RenderPassInstance;
	std::vector<PGPUImage> m_LocalRenderTargets;
	void RecordRenderPassInstance();
};

class GraphicsRenderPass
{
public:
	PRenderPass m_RenderPass;
	void InstantiateRenderPass(GraphicsRenderPassInstance &instance);
};