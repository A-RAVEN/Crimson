#pragma once
#include <headers/System/System.h>
#include <headers/Entity/Entity.h>
#include <set>

class HierarchyComp;
class TransformSystem : public ISystem
{
public:
	virtual void Configure(World &world) override;
	virtual void Execute(World const& world) override;
	virtual void OnEntityCreated(World const& world, EntityId entityId) override;
	virtual void OnEntityDestroyed(World const& world, EntityId entityId) override;
	static void DetachHierarchy(World const& world, HierarchyComp* this_hierarchy);
private:
	std::set<EntityId> const* p_TransformEntitySet;
	std::set<EntityId> const* p_HierarchyEntitySet;

	std::set<uint32_t> m_RootHierarchies;
};