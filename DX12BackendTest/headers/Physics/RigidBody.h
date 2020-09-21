#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace glm;
class RigidBodyFrame
{
public:
	vec3 m_Position;
	quat m_Rotation;
	vec3 m_LinearMomentum;
	vec3 m_AngularMomentum;
	RigidBodyFrame() :
		m_Position(0.0f),
		m_Rotation(vec3(0.0f)),
		m_LinearMomentum(0.0f),
		m_AngularMomentum(0.0f)
	{}
	void Reset() {
		m_Position = vec3(0.0f);
		m_Rotation = quat(vec3(0.0f));
		m_LinearMomentum = vec3(0.0f);
		m_AngularMomentum = vec3(0.0f);
	}
};