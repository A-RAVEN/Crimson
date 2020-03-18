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
