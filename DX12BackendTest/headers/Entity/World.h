#pragma once
#include <vector>
#include <headers/Entity/Entity.h>
#include <headers/Components/IComponentManager.h>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <bitset>
#include <headers/KeyboardController.h>
#include <headers/TimeManager.h>

class ISystem;
class World
{
public:
	World(TimeManager &time_manager, KeyboardController &controller);
	~World();
	void EnqueueComponentManager(IComponentManager* new_manager);
	void EnqueueSystem(ISystem* new_system);
	void Update();
	uint32_t GetComponentManagerId(std::string const& name) const;
	ComponentBits GetComponentManagerBitset(std::vector<std::string> const& names);
	IComponentManager* GetComponentManager(uint32_t comp_manager_id) const;
	IComponentManager* GetComponentManager(std::string const& comp_manager_name);

	EntityId AllocateEntity();
	void AddEntity(EntityId entity_id);
	void DestroyEntity(EntityId entity_id);
	IComponent* AddEntityComponent(EntityId entity_id, uint32_t comp_manager_id);
	IComponent* GetEntityComponent(EntityId entity_id, uint32_t comp_manager_id) const;
	IComponent* GetComponent(uint32_t comp_manager_id, uint32_t comp_id) const;

	TimeManager* GetTimeManager() { return m_TimeManager; }
	KeyboardController* GetController() {
		return m_Controller;
	}

	template <typename T>
	T* AddEntityComponent(EntityId entity_id)
	{
		static_assert(std::is_base_of<IComponent, T>::value, "type T should be derived from class IComponent!");
		return static_cast<T*>(AddEntityComponent(entity_id, GetComponentManagerId(std::string(typeid(T).name())) ));
	}

	template <typename T>
	T* GetEntityComponent(EntityId entity_id) const
	{
		static_assert(std::is_base_of<IComponent, T>::value, "type T should be derived from class IComponent!");
		return static_cast<T*>(GetEntityComponent(entity_id, GetComponentManagerId(std::string(typeid(T).name()))));
	}

	template <typename T>
	T* GetComponent(uint32_t comp_id) const
	{
		static_assert(std::is_base_of<IComponent, T>::value, "type T should be derived from class IComponent!");
		return static_cast<T*>(GetComponent(GetComponentManagerId(std::string(typeid(T).name())), comp_id) );
	}


	//Register a filter
	std::set<EntityId> const* RegisterFilter(ComponentBits const& bitset);

	template<class T>
	void Setbit(ComponentBits& bits)
	{
		static_assert(std::is_base_of<IComponent, T>::value, "type T should be derived from class IComponent!");
		bits.set(GetComponentManagerId(std::string(typeid(T).name())));
	}

	//variadic template pack expansion explain
	//	{ 0, ((void)bar(std::forward<Args>(args)), 0) ... };
	//	|       |       |                        |     |
	//	|       |       |                        |     ---pack expand the whole thing
	//	|       |       |                        |
	//	|       |       --perfect forwarding	 ---comma operator
	//	|       |
	//	|       --cast to void to ensure that regardless of bar()'s return type
	//	|		the built - in comma operator is used rather than an overloaded one
	//	|
	//	--- ensure that the array has at least one element so that we don't try to make an
	//		illegal 0 - length array when args is empty

	template <class ...Types>
	std::set<EntityId> const* RegisterFilter()
	{
		ComponentBits bits;
		int dummy[] = { 0, ((void)Setbit<Types>(bits), 0)... };
		return RegisterFilter(bits);
	}

	template <class ...Types>
	ComponentBits GetBits()
	{
		ComponentBits bits;
		int dummy[] = { 0, ((void)Setbit<Types>(bits), 0)... };
		return bits;
	}
private:
	KeyboardController* m_Controller;
	TimeManager* m_TimeManager;

	std::unordered_map<ComponentBits, size_t> m_EntityFilterReferences;
	std::deque<std::set<EntityId>> m_FilteredEntityLists;

	std::unordered_set<uint32_t> m_AvailableEntityIds;
	std::vector<Entity> m_Entities;

	std::unordered_map<std::string, uint32_t> m_ComponentManagerNameMap;
	std::vector<IComponentManager*> m_ComponentManagers;

	std::vector<ISystem*> m_Systems;
};