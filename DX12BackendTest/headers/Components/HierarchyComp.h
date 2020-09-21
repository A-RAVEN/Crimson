#pragma once
#include <headers/Components/IComponent.h>
#include <headers/Entity/Entity.h>
#include <set>
#include <glm/glm.hpp>
using namespace glm;
class HierarchyComp : public IComponent
{
public:
	virtual void ResetComponent() override
	{
		m_ParentHierarchy = InvalidEntityId;
		m_ChildHierarchies.clear();
		m_CachedTransform = mat4(1.0f);
		m_Dirty = true;
	}
	uint32_t m_ParentHierarchy;
	std::set<uint32_t> m_ChildHierarchies;
	mat4 m_CachedTransform;
	bool m_Dirty;
};