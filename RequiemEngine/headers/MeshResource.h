#pragma once
#include <assimp/scene.h>
#include <GPUDevice.h>
#include <Generals.h>
#include <headers/VertexData.h>
#include <PipelineEnums.h>
#include <glm/glm.hpp>

using namespace Crimson;
using namespace glm;
class MeshResource
{
public:
	template <typename VertexType>
	void ProcessAiScene(const aiScene* scene, bool constant_buffer = true)
	{
		std::vector<VertexType> vertices;
		std::vector<vec3> positions;
		std::vector<uint32_t> indicies;
		uint32_t submesh_vertex_offset = 0;
		for (uint64_t imesh = 0; imesh < scene->mNumMeshes; ++imesh)
		{
			const aiMesh* pmesh = scene->mMeshes[imesh];
			for (uint64_t ivertex = 0; ivertex < pmesh->mNumVertices; ++ivertex)
			{
				VertexType new_vertex{};
				new_vertex.m_Position = vec3(pmesh->mVertices[ivertex].x, pmesh->mVertices[ivertex].y, pmesh->mVertices[ivertex].z);
				vec3 i_normal = vec3(pmesh->mNormals[ivertex].x, pmesh->mNormals[ivertex].y, pmesh->mNormals[ivertex].z);
				new_vertex.m_Normal = vec3(pmesh->mNormals[ivertex].x, pmesh->mNormals[ivertex].y, pmesh->mNormals[ivertex].z);
				//new_vertex.m_Normal = vec3(1.0f, 0.0f, 0.0f);
				vec3 o_normal = new_vertex.m_Normal;
				if (pmesh->HasTangentsAndBitangents())
				{
					vec4 constructed_bt = vec4(pmesh->mTangents[ivertex].x, pmesh->mTangents[ivertex].y, pmesh->mTangents[ivertex].z, 0.0f);
					constructed_bt.w = glm::sign(
						glm::dot(glm::cross(vec3(new_vertex.m_Normal), vec3(constructed_bt)),
							vec3(pmesh->mBitangents[ivertex].x, pmesh->mBitangents[ivertex].y, pmesh->mBitangents[ivertex].z))
					);
					new_vertex.m_Tangent = constructed_bt;
				}
				if (pmesh->HasTextureCoords(0))
				{
					new_vertex.m_Texcoord = vec2(pmesh->mTextureCoords[0]->x, pmesh->mTextureCoords[0]->y);
				}
				vertices.push_back(new_vertex);
				vec3 new_pos = vec3(pmesh->mVertices[ivertex].x, pmesh->mVertices[ivertex].y, pmesh->mVertices[ivertex].z);
				positions.push_back(new_pos);
			}
			for (uint64_t iface = 0; iface < pmesh->mNumFaces; ++iface)
			{
				const aiFace& face = pmesh->mFaces[iface];
				for (uint8_t iid = 0; iid < face.mNumIndices; ++iid)
				{
					indicies.push_back(face.mIndices[iid] + submesh_vertex_offset);
				}
			}
			submesh_vertex_offset = vertices.size();
		}
		m_VertexSize = positions.size();
		m_IndexSize = indicies.size();
		PGPUDevice main_device = GPUDeviceManager::Get()->GetDevice("MainDevice");
		if (main_device)
		{
			size_t vertex_size = vertices.size() * sizeof(VertexType);
			size_t index_size = indicies.size() * sizeof(uint32_t);

			std::vector<EBufferUsage> vertex_buffer_usages = { EBufferUsage::E_BUFFER_USAGE_VERTEX };
			std::vector<EBufferUsage> index_buffer_usages = { EBufferUsage::E_BUFFER_USAGE_INDICIES };
			if (constant_buffer)
			{
				vertex_buffer_usages.push_back(EBufferUsage::E_BUFFER_USAGE_STORAGE);
				index_buffer_usages.push_back(EBufferUsage::E_BUFFER_USAGE_STORAGE);
			}
			m_VertexBuffer = main_device->CreateBuffer(vertex_size, vertex_buffer_usages, EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE);
			m_IndexBuffer = main_device->CreateBuffer(index_size, index_buffer_usages, EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE);

			memcpy_s(m_VertexBuffer->GetMappedPointer(), vertex_size, vertices.data(), vertex_size);
			memcpy_s(m_IndexBuffer->GetMappedPointer(), index_size, indicies.data(), index_size);
			m_VertexBuffer->UnMapp();
			m_IndexBuffer->UnMapp();
		}
		m_VertexStride = sizeof(VertexType);
		m_VertexDataType = VertexType::GetDataType();
	}

	void ProcessAiSceneLightWeight(const aiScene* scene, bool constant_buffer = true, bool packed = false);

	std::vector<EDataType> const& GetDataType() { return m_VertexDataType; }
	uint64_t GetVertexStride() { return m_VertexStride; }
public:
	PGPUBuffer m_VertexBuffer;
	PGPUBuffer m_IndexBuffer;
	uint32_t m_VertexSize;
	uint32_t m_IndexSize;

	uint64_t m_VertexStride;
	std::vector<EDataType> m_VertexDataType;
};