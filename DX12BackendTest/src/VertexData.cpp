#include <headers/VertexData.h>

bvec4_sn& bvec4_sn::operator=(vec4 const& src)
{
	r = static_cast<int8>(src.r * 127);
	g = static_cast<int8>(src.g * 127);
	b = static_cast<int8>(src.b * 127);
	a = static_cast<int8>(src.a * 127);
	return *this;
}

bvec4_sn& bvec4_sn::operator=(vec3 const& src)
{
	r = static_cast<int8>(src.r * 127);
	g = static_cast<int8>(src.g * 127);
	b = static_cast<int8>(src.b * 127);
	return *this;
}

bvec4_sn::operator vec4() const
{
	return vec4{
		static_cast<float>(r) / 127.0f,
		static_cast<float>(g) / 127.0f,
		static_cast<float>(b) / 127.0f,
		static_cast<float>(a) / 127.0f
	};
}

bvec4_sn::operator vec3() const
{
	return vec3{
		static_cast<float>(r) / 127.0f,
		static_cast<float>(g) / 127.0f,
		static_cast<float>(b) / 127.0f
	};
}
