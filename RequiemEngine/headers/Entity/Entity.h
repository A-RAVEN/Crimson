#pragma once
#include <vector>
#include <bitset>

using EntityId = uint32_t;
using ComponentBits = std::bitset<128>;
constexpr EntityId InvalidEntityId = (std::numeric_limits<uint32_t>::max)();

class Entity
{
public:
	friend class World;
private:
	void ResetEntity(uint32_t total_comp_size);
	void SetEntityComponet(uint32_t comp_manager_type, uint32_t component_id);
	bool HasComponent(uint32_t comp_manager_type) const;
	std::vector<uint32_t> m_ComponentMap;
	std::vector<uint32_t> m_ComponentTypes;
	ComponentBits m_ComponentBits;
};