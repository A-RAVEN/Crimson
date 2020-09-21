#include <headers/MeshResource.h>
#include <glm/glm.hpp>
#include <vector>

using namespace glm;

void MeshResource::ProcessAiSceneLightWeight(const aiScene* scene, bool constant_buffer, bool packed)
{
	if (packed)
	{
		ProcessAiScene<VertexDataLightWeight>(scene, constant_buffer);
	}
	else
	{
		ProcessAiScene<VertexDataLightWeightUnpacked>(scene, constant_buffer);
	}
}

AABB MeshResource::GetBox(mat4 transform)
{
	vec3 vecs[] =
	{
		m_InitialBoundingBox.m_LowerBound,
		m_InitialBoundingBox.m_UpperBound,
		vec3(m_InitialBoundingBox.m_LowerBound.x, m_InitialBoundingBox.m_UpperBound.y, m_InitialBoundingBox.m_LowerBound.z),
		vec3(m_InitialBoundingBox.m_UpperBound.x, m_InitialBoundingBox.m_LowerBound.y, m_InitialBoundingBox.m_LowerBound.z),
		vec3(m_InitialBoundingBox.m_UpperBound.x, m_InitialBoundingBox.m_UpperBound.y, m_InitialBoundingBox.m_LowerBound.z),
		vec3(m_InitialBoundingBox.m_LowerBound.x, m_InitialBoundingBox.m_UpperBound.y, m_InitialBoundingBox.m_UpperBound.z),
		vec3(m_InitialBoundingBox.m_UpperBound.x, m_InitialBoundingBox.m_LowerBound.y, m_InitialBoundingBox.m_UpperBound.z),
		vec3(m_InitialBoundingBox.m_LowerBound.x, m_InitialBoundingBox.m_LowerBound.y, m_InitialBoundingBox.m_UpperBound.z),
	};
	for (int i = 0; i < 8; ++i)
	{
		vecs[i] = transform * vec4(vecs[i].x, vecs[i].y, vecs[i].z, 1.0f);
	}
	AABB return_val;
	return_val.m_LowerBound = return_val.m_UpperBound = vecs[0];
	for (int i = 1; i < 8; ++i)
	{
		return_val.m_LowerBound = glm::min(return_val.m_LowerBound, vecs[i]);
		return_val.m_UpperBound = glm::max(return_val.m_UpperBound, vecs[i]);
	}
	return return_val;
}
