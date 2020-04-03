#pragma once
#include <GPUDevice.h>
#include <Generals.h>
#include <Pipeline.h>
#include <DescriptorSets.h>
using namespace Crimson;

class ImguiContext
{
public:
	//ImguiContext();
	//void Free();
	void SetDrawFrame(PGPUImage _frame);
	//void DrawFrame(bool draw);
	void InitContext(PGPUDevice device);
	//void ReleaseContext();
private:
	PGPUBuffer m_UniformBuffer;
	PGPUBuffer m_VerticesBuffer;
	PGPUBuffer m_IndicesBuffer;
	PFramebuffer m_Framebuffer;
	PDescriptorSetLayout m_Layout;
	PDescriptorSet m_Set;

	uint32_t m_VertexSize;
	uint32_t m_IndicesSize;
	bool m_Inited;

	PGPUDevice m_Device;
	PGraphicsPipeline m_ImguiPipeline;
	PRenderPass m_ImguiRenderPass;
	PRenderPassInstance m_ImguiRenderPassInstance;
};

class ImguiInstance
{
public:

private:
	ImguiContext* p_Context;
	PFramebuffer m_FrameBuffer;
	PGPUImage m_BaseImage;
};
