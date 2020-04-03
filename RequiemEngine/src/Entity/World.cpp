#include <headers/Entity/World.h>
#include <headers/Debug.h>
#include <headers/Components/TransformComponent.h>
World::World()
{
	EnqueueComponentManager("TransformComponent", new BuiltInComponentManager<TransformComp>());
}
World::~World()
{
	m_Entities.clear();
	m_ComponentManagerNameMap.clear();
	for (IComponentManager* itr_manager : m_ComponentManagers)
	{
		delete itr_manager;
	}
	m_ComponentManagers.clear();
}
void World::EnqueueComponentManager(std::string const& name, IComponentManager* new_manager)
{
	auto found = m_ComponentManagerNameMap.find(name);
	RE_ASSERT(found == m_ComponentManagerNameMap.end(), std::string("Component ") + name + "is enqueued twice!");
	m_ComponentManagerNameMap.insert(std::make_pair(name, m_ComponentManagers.size()));
	m_ComponentManagers.push_back(new_manager);
}

uint32_t World::GetComponentManagerId(std::string const& name)
{
	auto found = m_ComponentManagerNameMap.find(name);
	RE_ASSERT(found != m_ComponentManagerNameMap.end(), std::string("Component manager " + name + " not found!"));
	return found->second;
}

IComponentManager* World::GetComponentManager(uint32_t comp_manager_id)
{
	RE_ASSERT(comp_manager_id < m_ComponentManagers.size(), std::string("Invalid component manager id ") + std::to_string(comp_manager_id));
	return m_ComponentManagers[comp_manager_id];
}

IComponentManager* World::GetComponentManager(std::string const& comp_manager_name)
{
	return GetComponentManager(GetComponentManagerId(comp_manager_name));
}

EntityId World::AllocateEntity()
{
	if (!m_AvailableEntityIds.empty())
	{
		uint32_t return_val = *(m_AvailableEntityIds.begin());
		m_Entities[return_val].ResetEntity(m_ComponentManagers.size());
		m_AvailableEntityIds.erase(return_val);
		return return_val;
	}
	m_Entities.push_back(Entity{});
	m_Entities.back().ResetEntity(m_ComponentManagers.size());
	return m_Entities.size() - 1;
}

IComponent* World::AddEntityComponent(EntityId entity_id, uint32_t comp_manager_id)
{
	RE_ASSERT(entity_id < m_Entities.size(), "Invalid EntityId " + std::to_string(entity_id));
	RE_ASSERT(comp_manager_id < m_ComponentManagers.size(), "Invalid Component Manager Id " + std::to_string(comp_manager_id));
	Entity& entity = m_Entities[entity_id];
	RE_ASSERT(!entity.HasEntity(comp_manager_id), "Entity " + std::to_string(entity_id) + " already has component " + std::to_string(comp_manager_id));
	size_t component_id = m_ComponentManagers[comp_manager_id]->AllocateComponentId();
	entity.SetEntityComponet(comp_manager_id, component_id);
	return m_ComponentManagers[comp_manager_id]->GetComponent(component_id);
}

void World::DestroyEntity(EntityId entity_id)
{
	auto found = m_AvailableEntityIds.find(entity_id);
	if (found == m_AvailableEntityIds.end())
	{
		Entity& entity = m_Entities[entity_id];
		for (auto comp_manager_type : entity.m_ComponentTypes)
		{
			IComponentManager* manager = m_ComponentManagers[comp_manager_type];
			size_t component_id = entity.m_ComponentMap[comp_manager_type];
			manager->FreeComponentId(component_id);
		}
		m_AvailableEntityIds.insert(entity_id);
	}
}
