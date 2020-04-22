#pragma once
#include <headers/MeshResource.h>

class ResourceManager
{
public:
	MeshResource* GetMeshResource(std::string const& mesh_name);
};