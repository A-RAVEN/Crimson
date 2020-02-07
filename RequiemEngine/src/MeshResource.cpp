#include <headers/MeshResource.h>
#include <GPUDevice.h>
#include <glm/glm.hpp>
#include <vector>

using namespace glm;
void MeshResource::ProcessAiScene(const aiScene* scene)
{
	std::vector<vec3> positions;
	std::vector<uint32_t> indicies;
	for (uint64_t imesh = 0; imesh < scene->mNumMeshes; ++imesh)
	{
		const aiMesh* pmesh = scene->mMeshes[imesh];
		for (uint64_t ivertex = 0; ivertex < pmesh->mNumVertices; ++ivertex)
		{
			 vec3 new_pos = vec3(pmesh->mVertices[ivertex].x, pmesh->mVertices[ivertex].y, pmesh->mVertices[ivertex].z);
			 positions.push_back(new_pos);
		}
		for (uint64_t iface = 0; iface < pmesh->mNumFaces; ++iface)
		{
			const aiFace& face = pmesh->mFaces[iface];
			for (uint8_t iid = 0; iid < face.mNumIndices; ++iid)
			{
				indicies.push_back(face.mIndices[iid]);
			}
		}
	}
	m_VertexSize = positions.size();
	m_IndexSize = indicies.size();
	PGPUDevice main_device = GPUDeviceManager::Get()->GetDevice("MainDevice");
	if (main_device)
	{
		size_t vertex_size = positions.size() * sizeof(vec3);
		size_t index_size = indicies.size() * sizeof(uint32_t);

		m_VertexBuffer = main_device->CreateBuffer(vertex_size, { EBufferUsage::E_BUFFER_USAGE_VERTEX }, EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE);
		m_IndexBuffer = main_device->CreateBuffer(index_size, { EBufferUsage::E_BUFFER_USAGE_INDICIES }, EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE);

		memcpy_s(m_VertexBuffer->GetMappedPointer(), vertex_size, positions.data(), vertex_size);
		memcpy_s(m_IndexBuffer->GetMappedPointer(), index_size, indicies.data(), index_size);
		m_VertexBuffer->UnMapp();
		m_IndexBuffer->UnMapp();
	}
}
