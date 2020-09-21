#pragma once
#include <string>
#include <Generals.h>
#include <assimp/scene.h>
#include <GPUDevice.h>
#include <PipelineEnums.h>

struct MeshletDescriptor {
    uint32_t m_VertexCount; // number of vertices used
    uint32_t m_PrimitiveCount;   // number of primitives (triangles) used
    uint32_t m_VertexBegin; // offset into vertexIndices
    uint32_t m_PrimitiveBegin;   // offset into primitiveIndices
};

using namespace Crimson;
class MeshletGroupResource
{
public:
    MeshletGroupResource(PGPUDevice device, PDescriptorSetLayout setLayout);
	void LoadMeshRaw(const aiScene* scene);

    PGPUBuffer m_VertexBuffer;
    PGPUBuffer m_MeshletDescriptorBuffer;
    PGPUBuffer m_MeshletVertexIndexBuffer;//index of vertex used by each meshlet
    PGPUBuffer m_MeshletPrimitiveIndexBuffer;//local triangle indicies of each meshlet

    uint32_t m_MeshletSize;

    uint32_t m_VertexSize;
    uint32_t m_IndexSize;

    uint64_t m_VertexStride;
    std::vector<EDataType> m_VertexDataType;


    static PDescriptorSetLayout CreateSharedSetLayout(PGPUDevice srcDevice);

    PDescriptorSet GetDescriptorSet() const { return meshletSet; }
    PDescriptorSetLayout GetDescriptorSetLayout() const { return sharedSetLayout; }

private:
    PGPUDevice pDevice;
    PDescriptorSetLayout sharedSetLayout;

    PDescriptorSet meshletSet;
};