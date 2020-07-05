#pragma once
#include <headers/ThreadManager.h>
#include <headers/Camera.h>
#include <headers/MeshQueue.h>
#include <headers/GeometryInstanceManager.h>
#include <unordered_map>
#include <GPUDevice.h>
#include <Pipeline.h>
#include <mutex>
#include <headers/TimeManager.h>
#include <headers/Containers/ReaderWriterQueue/readerwriterqueue.h>
#include  <headers/Resources/MeshletResource.h>
#include <headers/AccelStructs/BVH.h>
#include <headers/BufferVector.h>

using namespace Crimson;
class RenderingSystem;
class RenderingSubThread : public ThreadJob
{
public:
	virtual void Work(ThreadWorker const* this_worker) override;
	void Init(RenderingSystem const*  rendering_system, PGPUDevice device);
	void PushInstanceQueue(MeshResource*, InstanceInfo const& info);
private:
	RenderingSystem const* p_RenderingSystem;
	PGPUDevice m_Device;
	PGPUDeviceThread m_DeviceThreadHandle;
	std::unordered_map<MeshResource*, MeshInstanceQueue> m_Instances;
};

class RenderingSystem : public ThreadJob
{
public:
	virtual void Work(ThreadWorker const* this_worker) override;
	RenderingSystem(IWindow* window, PAccelerationStructure blas,  PAccelerationStructure tlas, PGPUBuffer instance_buffer, MeshResource* rt_mesh, BufferQueue<uint32_t, 10> const& transform_queue, MeshletGroupResource* p_meshlets, MeshResource* p_cube);
	void SetupSystem();
	void UnInstallSystem();
	void SetupMeshletPipeline(PGPUDevice device, MeshletGroupResource *meshlet, PRenderPass renderpass);
	void PushBackNewFrame(GraphicsFrame &frame);
	void ParseBVH(BVH& bvh);
	TransformManager m_TransformManager;
	std::unordered_map<MeshResource*, MeshInstanceQueue> m_Instances;
	float DeltaTimeApprox() const { return m_AverageDeltaTime; }
	friend class RenderingSubThread;
private:
	bool TryPopFrame(GraphicsFrame& frame);
	std::mutex m_FrameQueueLock;
	moodycamel::ReaderWriterQueue<GraphicsFrame, 2> m_FrameQueue;

	TimeManager m_TimeManager;

	float m_AverageDeltaTime;

	PExecutionCommandBuffer m_LoadingBuffer;

	int m_Inited = 2;
	PGPUDeviceThread m_RenderingThread;
	PGPUBuffer m_LastCameraBuffer;
	PGPUBuffer m_CameraBuffer;
	PGPUBuffer m_TimeBuffer;
	PDescriptorSetLayout m_SetLayout;
	PDescriptorSet m_Set;
	PGraphicsPipeline m_Pipeline;
	PDescriptorSetLayout m_MeshletSetLayout;
	PDescriptorSet m_MeshletSet;
	PGraphicsPipeline m_MeshletPipeline;
	PRenderPassInstance m_RenderPassInstance;
	PExecutionCommandBuffer m_ExecutionCmd;
	PExecutionCommandBuffer m_PresentCmd;
	PGPUImage m_Color;
	PGPUImage m_Normal;
	IWindow* p_Window;

	PRayTracer m_RayTracer;
	PDescriptorSetLayout m_RtSetLayout;
	PDescriptorSet m_RtSet;
	PGPUBuffer m_ShaderTable;
	PGPUImage m_RTColor;
	PGPUImage m_RTColorOld;

	MeshletGroupResource* p_MeshletMesh;
	PGPUBuffer MeshletTransform;

	BVH TestBVH;

	MeshResource *p_CubeResource;
	BufferQueue<mat4, sizeof(mat4) * 20> m_CubeTransforms;

	PGraphicsPipeline FilterPipeline;
};