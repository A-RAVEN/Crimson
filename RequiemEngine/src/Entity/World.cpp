#include <headers/Entity/World.h>
#include <headers/Debug.h>
#include <headers/Components/TransformComponent.h>
#include <headers/Components/MeshRenderComp.h>
#include <headers/Components/HierarchyComp.h>
#include <headers/System/System.h>
#include <headers/System/TransformSystem.h>
World::World()
{
	//Enqueue components
	EnqueueComponentManager(new BuiltInComponentManager<TransformComp>());
	EnqueueComponentManager(new BuiltInComponentManager<MeshRenderComp>());
	EnqueueComponentManager(new BuiltInComponentManager<HierarchyComp>());

	//Enqueue systems after components
	EnqueueSystem(new TransformSystem());
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
void World::EnqueueComponentManager(IComponentManager* new_manager)
{
	std::string name = new_manager->GetName();
	auto found = m_ComponentManagerNameMap.find(name);
	RE_ASSERT(found == m_ComponentManagerNameMap.end(), std::string("Component ") + name + "is enqueued twice!");
	m_ComponentManagerNameMap.insert(std::make_pair(name, m_ComponentManagers.size()));
	m_ComponentManagers.push_back(new_manager);
}

void World::EnqueueSystem(ISystem* new_system)
{
	m_Systems.push_back(new_system);
	new_system->Configure(*this);
}

void World::Update()
{
	for (auto system : m_Systems)
	{
		system->Execute(*this);
	}
}

uint32_t World::GetComponentManagerId(std::string const& name) const
{
	auto found = m_ComponentManagerNameMap.find(name);
	RE_ASSERT(found != m_ComponentManagerNameMap.end(), std::string("Component manager " + name + " not found!"));
	return found->second;
}

ComponentBits World::GetComponentManagerBitset(std::vector<std::string> const& names)
{
	ComponentBits return_val = 0x0;
	for (auto& name : names)
	{
		return_val.set(GetComponentManagerId(name));
	}
	return return_val;
}

IComponentManager* World::GetComponentManager(uint32_t comp_manager_id) const
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

void World::AddEntity(EntityId entity_id)
{
	RE_ASSERT(entity_id < m_Entities.size(), "Invalid EntityId " + std::to_string(entity_id));
	Entity& entity = m_Entities[entity_id];
	for (auto p_systems : m_Systems)
	{
		if (p_systems->TestingBits(entity.m_ComponentBits))
		{
			p_systems->OnEntityCreated(*this, entity_id);
		}
	}
	for (auto& filter_reference : m_EntityFilterReferences)
	{
		if ((filter_reference.first & entity.m_ComponentBits) == filter_reference.first)
		{
			m_FilteredEntityLists[filter_reference.second].insert(entity_id);
		}
	}
}

IComponent* World::AddEntityComponent(EntityId entity_id, uint32_t comp_manager_id)
{
	RE_ASSERT(entity_id < m_Entities.size(), "Invalid EntityId " + std::to_string(entity_id));
	RE_ASSERT(comp_manager_id < m_ComponentManagers.size(), "Invalid Component Manager Id " + std::to_string(comp_manager_id));
	Entity& entity = m_Entities[entity_id];
	RE_ASSERT(!entity.HasComponent(comp_manager_id), "Entity " + std::to_string(entity_id) + " already has component " + std::to_string(comp_manager_id));
	size_t component_id = m_ComponentManagers[comp_manager_id]->AllocateComponentId();
	entity.SetEntityComponet(comp_manager_id, component_id);
	IComponent* return_val = m_ComponentManagers[comp_manager_id]->GetComponent(component_id);
	return_val->m_Entity = entity_id;
	return_val->m_CompId = component_id;
	return return_val;
}

IComponent* World::GetEntityComponent(EntityId entity_id, uint32_t comp_manager_id) const
{
	RE_ASSERT(entity_id < m_Entities.size(), "Invalid EntityId " + std::to_string(entity_id));
	Entity const& entity = m_Entities[entity_id];
	if (entity.HasComponent(comp_manager_id))
	{
		IComponentManager* manager = GetComponentManager(comp_manager_id);
		return manager->GetComponent(entity.m_ComponentMap[comp_manager_id]);
	}
	return nullptr;
}

IComponent* World::GetComponent(uint32_t comp_manager_id, uint32_t comp_id) const
{
	IComponentManager* manager = GetComponentManager(comp_manager_id);
	return manager->GetComponent(comp_id);
}


void World::DestroyEntity(EntityId entity_id)
{
	auto found = m_AvailableEntityIds.find(entity_id);
	if (found == m_AvailableEntityIds.end())
	{
		Entity& entity = m_Entities[entity_id];
		for (auto p_systems : m_Systems)
		{
			if (p_systems->TestingBits(entity.m_ComponentBits))
			{
				p_systems->OnEntityDestroyed(*this, entity_id);
			}
		}
		for (auto& filter_reference : m_EntityFilterReferences)
		{
			if ((filter_reference.first & entity.m_ComponentBits) == filter_reference.first)
			{
				m_FilteredEntityLists[filter_reference.second].erase(entity_id);
			}
		}
		for (auto comp_manager_type : entity.m_ComponentTypes)
		{
			IComponentManager* manager = m_ComponentManagers[comp_manager_type];
			size_t component_id = entity.m_ComponentMap[comp_manager_type];
			manager->FreeComponentId(component_id);
		}
		m_AvailableEntityIds.insert(entity_id);
	}
}

std::set<EntityId> const* World::RegisterFilter(ComponentBits const& setkey)
{
	auto find = m_EntityFilterReferences.find(setkey);
	if (find == m_EntityFilterReferences.end())
	{
		m_FilteredEntityLists.push_back(std::set<EntityId>());
		m_EntityFilterReferences.insert(std::make_pair(setkey, m_FilteredEntityLists.size() - 1));
		return &m_FilteredEntityLists.back();
	}
	return &m_FilteredEntityLists[find->second];
}
