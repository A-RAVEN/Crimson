#include <headers/Resources/MeshletResource.h>
#include <headers/VertexData.h>
#include <set>

MeshletGroupResource::MeshletGroupResource(PGPUDevice device, PDescriptorSetLayout setLayout) : pDevice(device), sharedSetLayout(setLayout)
{
}

void MeshletGroupResource::LoadMeshRaw(const aiScene* scene)
{
	std::vector<VertexDataLightWeightUnpacked> vertices;
	std::vector<uint32_t> indicies;
	std::vector<vec3> positions;
	std::vector<uint32_t> meshlet_vertices;
	std::vector<uint8_t> primitives;
	std::vector<MeshletDescriptor> meshlet_descs;
	uint32_t submesh_vertex_offset = 0;
	for (uint64_t imesh = 0; imesh < scene->mNumMeshes; ++imesh)
	{
		const aiMesh* pmesh = scene->mMeshes[imesh];
		for (uint64_t ivertex = 0; ivertex < pmesh->mNumVertices; ++ivertex)
		{
			VertexDataLightWeightUnpacked new_vertex{};
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

	//split 
	{
		uint32_t max_meshlet_vert = 64;
		uint32_t max_meshlet_prim = 126;

		int index_id = 0;
		std::map<uint32_t, uint8_t> meshlet_local_vertices;
		std::map<uint8_t, uint32_t> inverse_meshlet_local_vertices;
		while (index_id < indicies.size()) 
		{
			// new meshlet
			meshlet_local_vertices.clear();
			inverse_meshlet_local_vertices.clear();
			uint8_t primitive_id = 0;
			MeshletDescriptor new_descriptor;
			new_descriptor.m_VertexBegin = meshlet_vertices.size();
			new_descriptor.m_PrimitiveBegin = primitives.size() / 3;
			new_descriptor.m_PrimitiveCount = 0;
			new_descriptor.m_VertexCount = 0;
			for (int i = 0; i < max_meshlet_prim && index_id < indicies.size(); ++i)
			{
				int new_vert_count = 0;
				std::vector<uint32_t> new_indicies;
				for (int j = 0; j < 3; ++j)
				{
					uint32_t new_vert = indicies[index_id + j];
					if (meshlet_local_vertices.find(new_vert) == meshlet_local_vertices.end())
					{
						new_indicies.push_back(new_vert);
						++new_vert_count;
					}
				}
				if ((meshlet_local_vertices.size() + new_vert_count) <= max_meshlet_vert)
				{
					for (int j = 0; j < new_indicies.size(); ++j)
					{
						uint32_t new_vert = indicies[index_id + j];
						meshlet_local_vertices.insert(std::make_pair(new_indicies[j], primitive_id++));
					}
					for (int j = 0; j < 3; ++j)
					{
						uint8_t prim_id = meshlet_local_vertices[indicies[index_id + j]];
						primitives.push_back(prim_id);
					}
					index_id += 3;
					new_descriptor.m_PrimitiveCount += 1;
					new_descriptor.m_VertexCount += new_vert_count;
				}
				else
				{
					break;
				}
			}
			//add new 
			if (meshlet_local_vertices.size() > 0)
			{
				for (auto& pair : meshlet_local_vertices)
				{
					inverse_meshlet_local_vertices.insert(std::make_pair(pair.second, pair.first));
				}
				for (auto& pair : inverse_meshlet_local_vertices)
				{
					meshlet_vertices.push_back(pair.second);
				}
				meshlet_descs.push_back(new_descriptor);
			}
		}
	}

	m_MeshletSize = meshlet_descs.size();
	m_VertexSize = positions.size();
	m_IndexSize = indicies.size();
	m_VertexStride = sizeof(VertexDataLightWeightUnpacked);
	m_VertexDataType = VertexDataLightWeightUnpacked::GetDataType();

	//change to device injection later
	PGPUDevice main_device = pDevice;
	if (main_device)
	{
		size_t vertex_size = vertices.size() * sizeof(VertexDataLightWeightUnpacked);
		size_t meshlet_vertex_id_size = meshlet_vertices.size() * sizeof(uint32_t);
		size_t meshlet_prim_id_size = primitives.size() * sizeof(uint8_t);
		size_t meshlet_desc_size = m_MeshletSize * sizeof(MeshletDescriptor);

		std::vector<EBufferUsage> vertex_buffer_usages = { EBufferUsage::E_BUFFER_USAGE_VERTEX, EBufferUsage::E_BUFFER_USAGE_STORAGE, EBufferUsage::E_BUFFER_USAGE_STORAGE_TEXEL };

		std::vector<EBufferUsage> meshlet_vert_indices_usages = { EBufferUsage::E_BUFFER_USAGE_STORAGE_TEXEL };
		std::vector<EBufferUsage> meshlet_prim_indices_usages = { EBufferUsage::E_BUFFER_USAGE_STORAGE_TEXEL };

		m_VertexBuffer = main_device->CreateBuffer(vertex_size, vertex_buffer_usages, EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE); 
		m_VertexBuffer->InitTexelBufferView("default", EFormat::E_FORMAT_R32G32B32A32_SFLOAT);
		m_MeshletVertexIndexBuffer = main_device->CreateBuffer(meshlet_vertex_id_size, meshlet_vert_indices_usages, EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE);
		m_MeshletVertexIndexBuffer->InitTexelBufferView("default", EFormat::E_FORMAT_R32_UINT);
		m_MeshletPrimitiveIndexBuffer = main_device->CreateBuffer(meshlet_prim_id_size, meshlet_prim_indices_usages, EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE);
		m_MeshletPrimitiveIndexBuffer->InitTexelBufferView("default", EFormat::E_FORMAT_R8_UINT);
		m_MeshletDescriptorBuffer = main_device->CreateBuffer(meshlet_desc_size, { EBufferUsage::E_BUFFER_USAGE_STORAGE }, EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE);

		memcpy_s(m_VertexBuffer->GetMappedPointer(), vertex_size, vertices.data(), vertex_size);
		memcpy_s(m_MeshletVertexIndexBuffer->GetMappedPointer(), meshlet_vertex_id_size, meshlet_vertices.data(), meshlet_vertex_id_size);
		memcpy_s(m_MeshletPrimitiveIndexBuffer->GetMappedPointer(), meshlet_prim_id_size, primitives.data(), meshlet_prim_id_size);
		memcpy_s(m_MeshletDescriptorBuffer->GetMappedPointer(), meshlet_desc_size, meshlet_descs.data(), meshlet_desc_size);
		m_VertexBuffer->UnMapp();
		m_MeshletVertexIndexBuffer->UnMapp();
		m_MeshletPrimitiveIndexBuffer->UnMapp();
		m_MeshletDescriptorBuffer->UnMapp();

		//д descriptorSet
		meshletSet = sharedSetLayout->AllocDescriptorSet();
		meshletSet->WriteDescriptorSetTexelBufferView(0, m_MeshletVertexIndexBuffer, "default", 0);
		meshletSet->WriteDescriptorSetTexelBufferView(1, m_MeshletPrimitiveIndexBuffer, "default", 0);
		meshletSet->WriteDescriptorSetTexelBufferView(2, m_VertexBuffer, "default", 0);
		meshletSet->WriteDescriptorSetBuffers(3, { m_MeshletDescriptorBuffer }, { m_MeshletDescriptorBuffer->GetRange() }, 0);
		meshletSet->EndWriteDescriptorSet();
	}
}

PDescriptorSetLayout MeshletGroupResource::CreateSharedSetLayout(PGPUDevice srcDevice)
{
	PDescriptorSetLayout return_val = srcDevice->CreateDescriptorSetLayout();
	return_val->m_Bindings.resize(4);

	return_val->m_Bindings[0].m_ResourceType = EShaderResourceType::E_SHADER_STORAGE_TEXEL_BUFFER;
	return_val->m_Bindings[0].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_TASK_NV, EShaderType::E_SHADER_TYPE_MESH_NV };
	return_val->m_Bindings[0].m_BindingPoint = 0;
	return_val->m_Bindings[0].m_Num = 1;

	return_val->m_Bindings[1].m_ResourceType = EShaderResourceType::E_SHADER_STORAGE_TEXEL_BUFFER;
	return_val->m_Bindings[1].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_TASK_NV, EShaderType::E_SHADER_TYPE_MESH_NV };
	return_val->m_Bindings[1].m_BindingPoint = 1;
	return_val->m_Bindings[1].m_Num = 1;

	return_val->m_Bindings[2].m_ResourceType = EShaderResourceType::E_SHADER_STORAGE_TEXEL_BUFFER;
	return_val->m_Bindings[2].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_TASK_NV, EShaderType::E_SHADER_TYPE_MESH_NV };
	return_val->m_Bindings[2].m_BindingPoint = 2;
	return_val->m_Bindings[2].m_Num = 1;

	return_val->m_Bindings[3].m_ResourceType = EShaderResourceType::E_SHADER_TYPE_STORAGE_BUFFER;
	return_val->m_Bindings[3].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_TASK_NV, EShaderType::E_SHADER_TYPE_MESH_NV };
	return_val->m_Bindings[3].m_BindingPoint = 3;
	return_val->m_Bindings[3].m_Num = 1;
	return_val->BuildLayout();
	return return_val;
}
