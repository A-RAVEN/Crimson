#include <headers/System/TransformSystem.h>
#include <headers/Entity/World.h>
#include <headers/Components/TransformComponent.h>
#include <headers/Components/HierarchyComp.h>
#include <headers/Components/MeshRenderComp.h>
#include <algorithm>

void TransformSystem::Configure(World &world)
{
	p_TransformEntitySet = world.RegisterFilter<TransformComp>();
	p_HierarchyEntitySet = world.RegisterFilter<HierarchyComp>();
	ConfigureListeningBits({ world.GetBits<TransformComp>(), world.GetBits<HierarchyComp>() });
}

void TransformSystem::Execute(World const& world)
{
	//update transforms
	for (auto entity : (*p_TransformEntitySet))
	{
		TransformComp* comp = world.GetEntityComponent<TransformComp>(entity);
		if (comp->m_Dirty)
		{
			comp->m_CachedTransform = translate(mat4(1.0f), comp->m_Position) * toMat4(comp->m_Rotation) * scale(mat4(1.0f), comp->m_Scale);
		}
	}
	std::deque<HierarchyComp*> queue;
	for (auto hierarchy_id : m_RootHierarchies)
	{
		HierarchyComp* root = world.GetEntityComponent<HierarchyComp>(hierarchy_id);
		queue.push_back(root);
		while (!queue.empty())
		{
			HierarchyComp* comp = queue.front();
			queue.pop_front();
			bool parent_comp_dirty = false;
			bool transform_dirty = false;
			HierarchyComp* parent_comp = nullptr;
			TransformComp* transform = nullptr;
			if (comp->m_ParentHierarchy != InvalidEntityId)
			{
				parent_comp = world.GetEntityComponent<HierarchyComp>(comp->m_ParentHierarchy);
				parent_comp_dirty = parent_comp->m_Dirty;
			}
			transform = world.GetEntityComponent<TransformComp>(comp->GetEntityId());
			if (transform)
			{
				transform_dirty = transform->m_Dirty;
				transform->m_Dirty = true;
			}
			comp->m_Dirty = transform_dirty || parent_comp_dirty;
			if (comp->m_Dirty)
			{
				comp->m_CachedTransform = mat4(1.0f);
				if (transform)
				{
					comp->m_CachedTransform = transform->m_CachedTransform;
				}
				if (parent_comp)
				{
					comp->m_CachedTransform = parent_comp->m_CachedTransform * comp->m_CachedTransform;
				}
			}
			for (auto child : comp->m_ChildHierarchies)
			{
				queue.push_back(world.GetEntityComponent<HierarchyComp>(child));
			}
		}
	}

	for (auto entity : (*p_HierarchyEntitySet))
	{
		HierarchyComp* comp = world.GetEntityComponent<HierarchyComp>(entity);
		if (comp->m_Dirty)
		{
			comp->m_Dirty = true;
		}
	}
}

void TransformSystem::OnEntityCreated(World const& world, EntityId entityId)
{
	HierarchyComp* comp = world.GetEntityComponent<HierarchyComp>(entityId);
	if (comp != nullptr)
	{
		if (comp->m_ParentHierarchy == InvalidEntityId)
		{
			m_RootHierarchies.insert(comp->GetCompId());
		}
	}
}

void TransformSystem::OnEntityDestroyed(World const& world, EntityId entityId)
{
	HierarchyComp* comp = world.GetEntityComponent<HierarchyComp>(entityId);
	if (comp != nullptr)
	{
		if (comp->m_ParentHierarchy == InvalidEntityId)
		{
			m_RootHierarchies.erase(comp->GetCompId());
		}
		else
		{
			DetachHierarchy(world, comp);
		}
	}
}

void TransformSystem::DetachHierarchy(World const& world, HierarchyComp* this_hierarchy)
{
	if (this_hierarchy && this_hierarchy->m_ParentHierarchy != InvalidEntityId)
	{
		HierarchyComp* parent_hierarchy = world.GetComponent<HierarchyComp>(this_hierarchy->m_ParentHierarchy);
		if (parent_hierarchy)
		{
			parent_hierarchy->m_ChildHierarchies.erase(this_hierarchy->GetCompId());
		}
		this_hierarchy->m_ParentHierarchy = InvalidEntityId;
	}
}
