#version 450
#include "test.gh"

layout(location = 0) in vec3 positioin;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec4 normal_data;
layout(location = 3) in vec4 tangent_data;

layout(location = 4) in uint transform_id;

layout(location = 0) out vec4 out_normal;

void main()
{
	out_normal = normal_data;
	gl_Position = cam.proj * cam.view * transforms[transform_id] * vec4(positioin, 1.0);
}
