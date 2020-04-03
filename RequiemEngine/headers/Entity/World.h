#pragma once
#include <vector>
#include <headers/Entity/Entity.h>
#include <headers/Components/IComponentManager.h>
#include <unordered_map>
#include <unordered_set>
class World
{
public:
	World();
	~World();
	void EnqueueComponentManager(std::string const& name, IComponentManager* new_manager);
	uint32_t GetComponentManagerId(std::string const& name);
	IComponentManager* GetComponentManager(uint32_t comp_manager_id);
	IComponentManager* GetComponentManager(std::string const& comp_manager_name);

	EntityId AllocateEntity();
	IComponent* AddEntityComponent(EntityId entity_id, uint32_t comp_manager_id);
	void DestroyEntity(EntityId entity_id);

private:
	std::unordered_set<uint32_t> m_AvailableEntityIds;
	std::vector<Entity> m_Entities;

	std::unordered_map<std::string, uint32_t> m_ComponentManagerNameMap;
	std::vector<IComponentManager*> m_ComponentManagers;
};