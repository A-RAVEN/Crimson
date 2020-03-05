#pragma once
#include <headers/ThreadManager.h>
#include <headers/Camera.h>
#include <headers/MeshQueue.h>
#include <headers/GeometryInstanceManager.h>
#include <unordered_map>
#include <GPUDevice.h>
#include <Pipeline.h>
#include <mutex>

using namespace Crimson;
class RenderingSystem : public ThreadJob
{
public:
	virtual void Work(ThreadWorker const* this_worker) override;
	RenderingSystem(IWindow* window);
	void PushBackNewFrame(GraphicsFrame const& frame);
	TransformManager m_TransformManager;
	std::unordered_map<MeshResource*, MeshInstanceQueue> m_Instances;
private:
	bool TryPopFrame(GraphicsFrame& frame);
	std::mutex m_FrameQueueLock;
	std::list<GraphicsFrame> m_FrameQueue;

	PGPUDeviceThread m_RenderingThread;
	PGPUBuffer m_CameraBuffer;
	PDescriptorSetLayout m_SetLayout;
	PDescriptorSet m_Set;

	PGraphicsPipeline m_Pipeline;
	PRenderPassInstance m_RenderPassInstance;

	PExecutionCommandBuffer m_ExecutionCmd;

	PGPUImage m_Color;

	IWindow* p_Window;
};