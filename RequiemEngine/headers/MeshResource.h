#pragma once
#include <assimp/scene.h>
#include <Generals.h>

using namespace Crimson;
class MeshResource
{
public:
	void ProcessAiScene(const aiScene* scene, bool constant_buffer = true);
public:
	PGPUBuffer m_VertexBuffer;
	PGPUBuffer m_IndexBuffer;
	uint32_t m_VertexSize;
	uint32_t m_IndexSize;
};