#pragma once
#include <glm/glm.hpp>
#include <stdint.h>
#include <deque>
#include <headers/MeshResource.h>
#include <headers/Camera.h>
#include <map>

class TransformInfo
{
public:
	TransformInfo(glm::mat4 const& matrix, uint32_t batch_id, uint32_t transform_id) :
		m_Matrix(matrix),
		m_BatchId(batch_id),
		m_TransformId(transform_id)
	{}
	glm::mat4 m_Matrix;
	uint32_t m_BatchId;
	uint32_t m_TransformId;
};

class InstanceInfo
{
public:
	MeshResource* p_Mesh;
	uint32_t m_BatchId;
	uint32_t m_TransformId;
};

class GraphicsFrame
{
public:
	void AddInstance(InstanceInfo const& instance_info);
	void RemoveInstance(InstanceInfo const& instance_info);
	void UpdateTransform(TransformInfo const& transform_info);
	std::deque<TransformInfo> m_TransoformUpdateInfo;
	std::deque<InstanceInfo> m_RemovedInstances;
	std::map<MeshResource*, std::deque<InstanceInfo>> m_InstanceList;
	Camera m_Camera;
};