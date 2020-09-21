#pragma once
#include <headers/Components/IComponent.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;
class TransformComp : public IComponent
{
public:
	TransformComp() : m_Position(0.0f),
		m_Scale(1.0f),
		m_Rotation(0.0f, 0.0f, 0.0f, 0.0f),
		m_Dirty(true),
		m_CachedTransform(1.0f),
		m_ParentTransformComp((std::numeric_limits<uint32_t>::max)()) 
	{}
	vec3 m_Position;
	vec3 m_Scale;
	quat m_Rotation;
	
	bool m_Dirty;
	mat4 m_CachedTransform;
	uint32_t m_ParentTransformComp;
};