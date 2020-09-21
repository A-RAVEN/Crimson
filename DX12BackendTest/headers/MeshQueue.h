#pragma once
#include <headers/MeshResource.h>
#include <headers/GeometryInstanceManager.h>
#include <vector>
#include <deque>
#include <GPUDevice.h>
#include <headers/BufferVector.h>

class MeshInstanceData
{
public:
	uint32_t m_TransformId;
};

class MeshInstanceQueue
{
public:
	MeshResource *m_Resource;
	std::vector<BufferQueue<MeshInstanceData, 500>> MeshQueue;
	void PushInstance(InstanceInfo const& instance_info);
	void Clear();
	void CmdDrawInstances(PGraphicsCommandBuffer commane_buffer, uint32_t transform_batch_id);
};