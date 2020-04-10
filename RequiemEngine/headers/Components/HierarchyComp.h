#pragma once
#include <headers/Components/IComponent.h>
#include <headers/Entity/Entity.h>
#include <set>
#include <glm/glm.hpp>
using namespace glm;
class HierarchyComp : public IComponent
{
public:
	virtual void ResetComponent() override;
	uint32_t m_ParentHierarchy;
	std::set<uint32_t> m_ChildHierarchies;
	mat4 m_CachedTransform;
	bool m_Dirty;
};