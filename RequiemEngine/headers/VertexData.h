#pragma once
#include <PipelineEnums.h>
#include <Pipeline.h>
#include <vector>
#include <glm/glm.hpp>
using namespace glm;
using namespace Crimson;

class bvec4_sn
{
public:
	int8_t r;
	int8_t g;
	int8_t b;
	int8_t a;
	bvec4_sn() :
		r(0),g(0),b(0),a(0)
	{}
	bvec4_sn& operator=(vec4 const& src);
	bvec4_sn& operator=(vec3 const& src);
	operator vec4() const;
	operator vec3() const;
};

enum class EVertexType : uint8_t
{
	E_VERTEX_TYPE_LIGHTWEIGHT = 0,
	E_VERTEX_TYPE_MINIMUN,
	E_VERTEX_TYPE_EXTENDED,
};

class VertexDataLightWeight
{
public:
	vec3 m_Position;
	vec2 m_Texcoord;
	bvec4_sn m_Normal;
	bvec4_sn m_Tangent;
	static std::vector<EDataType> GetDataType()
	{
		return { EDataType::EVEC3, EDataType::EVEC2, EDataType::ERGBA8_SNORM, EDataType::ERGBA8_SNORM };
	}

	static VertexInputDescriptor GetInputDescriptor()
	{
		return { EVertexInputMode::E_VERTEX_INPUT_PER_VERTEX, GetDataType() };
	}
};

class VertexDataLightWeightUnpacked
{
public:
	vec3 m_Position;
	vec2 m_Texcoord;
	vec3 m_Normal;
	vec4 m_Tangent;
	static std::vector<EDataType> GetDataType()
	{
		return { EDataType::EVEC3, EDataType::EVEC2, EDataType::EVEC3, EDataType::EVEC4 };
	}
};

class VertexDataMinimun
{
public:
	vec3 m_Position;
};

class VertexDataExtended
{
public:
	vec3 m_Position;
	vec2 m_Texcoord;
	bvec4_sn m_Normal;
	bvec4_sn m_Tangent;
	bvec4 m_Color;
};

class VertexDataPresets
{
public:
	static VertexInputDescriptor INSTANCE_INDEX() { return { EVertexInputMode::E_VERTEX_INPUT_PER_INSTANCE, {EDataType::EUINT} }; }
};