#pragma once
#include <headers/Components/IComponent.h>
#include <vector>
class MeshRenderComp : public IComponent
{
public:
	uint32_t m_MeshResourceId;
	bool m_SingleMaterial;
	std::vector<uint32_t> m_MaterialReferences;
};