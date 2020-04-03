#pragma once
#include <vector>

class Entity
{
public:
	friend class World;
private:
	void ResetEntity(uint32_t total_comp_size);
	void SetEntityComponet(uint32_t comp_manager_type, uint32_t component_id);
	bool HasEntity(uint32_t comp_manager_type);
	std::vector<uint32_t> m_ComponentMap;
	std::vector<uint32_t> m_ComponentTypes;
};

using EntityId = uint32_t;