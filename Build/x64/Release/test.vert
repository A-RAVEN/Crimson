#version 450
#include "camera.gh"

layout(set = 0, binding = 0) uniform MATRIX
{
	Camera cam;
};

layout(location = 0) in vec3 positioin;

void main()
{
	gl_Position = cam.proj * cam.view * vec4(positioin, 1.0);
}
