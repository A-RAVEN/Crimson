#include <headers/Entity/Entity.h>
#include <headers/Debug.h>
void Entity::ResetEntity(uint32_t total_comp_size)
{
	m_ComponentBits = 0x0;
	m_ComponentMap.resize(total_comp_size);
	size_t size = m_ComponentMap.size();
	uint32_t num_max = (std::numeric_limits<uint32_t>::max)();
	for (size_t i = 0; i < size; ++i)
	{
		m_ComponentMap[i] = num_max;
	}
	m_ComponentTypes.clear();
}
void Entity::SetEntityComponet(uint32_t component_type, uint32_t component_id)
{
	RE_ASSERT(component_type < m_ComponentMap.size(), "Invalid component type id " + std::to_string(component_type));
	m_ComponentTypes.push_back(component_type);
	m_ComponentMap[component_type] = component_id;
	m_ComponentBits.set(component_type, true);
}
bool Entity::HasComponent(uint32_t component_type) const
{
	RE_ASSERT(component_type < m_ComponentMap.size(), "Invalid component type id " + std::to_string(component_type));
	return m_ComponentMap[component_type] != (std::numeric_limits<uint32_t>::max)();
}
