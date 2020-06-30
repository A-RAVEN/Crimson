#include <headers/AccelStructs/AABB.h>

float AABB::Area()
{
	vec3 d = m_UpperBound - m_LowerBound;
	return 2.0f * (d.x * d.y + d.y * d.z + d.z * d.x);
}

AABB AABB::Union(AABB const& b1, AABB const& b2)
{
	return AABB{glm::min(b1.m_LowerBound, b2.m_LowerBound), glm::max(b1.m_UpperBound, b2.m_UpperBound)};
}
