#pragma once
#include <glm/glm.hpp>
using namespace glm;

class AABB
{
public:
	vec3 m_LowerBound;
	vec3 m_UpperBound;
	AABB() : m_LowerBound(0.0f), m_UpperBound(1.0f) {}
	AABB(vec3 const& lower_bound, vec3 const& upper_bound) : m_LowerBound(lower_bound), m_UpperBound(upper_bound) {}
	vec3 Center() { return (m_LowerBound + m_UpperBound) * 0.5f; }
	float Area();
	static AABB Union(AABB const& b1, AABB const& b2);
};