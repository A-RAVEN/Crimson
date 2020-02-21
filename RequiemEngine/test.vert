#version 450
#include "camera.gh"

layout(set = 0, binding = 0) uniform MATRIX
{
	Camera cam;
};

layout(location = 0) in vec3 positioin;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec4 normal_data;
layout(location = 3) in vec4 tangent_data;

layout(location = 0) out vec4 out_normal;

void main()
{
	out_normal = normal_data;
	gl_Position = cam.proj * cam.view * vec4(positioin, 1.0);
}
