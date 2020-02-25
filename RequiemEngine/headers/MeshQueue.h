#pragma once
#include <headers/MeshResource.h>
#include <vector>
#include <deque>
#include <GPUDevice.h>
#include <headers/BufferVector.h>

class MeshInstanceData
{
	uint32_t m_TransformId;
};

class MeshInstanceQueue
{
public:
	MeshResource *m_Resource;
	std::vector<BufferQueue<MeshInstanceData, 500>> MeshQueue;
	void CmdDrawInstances(PGraphicsCommandBuffer commane_buffer, uint32_t transform_batch_id);
};