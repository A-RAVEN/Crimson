#version 450

layout(location = 0) in vec3 positioin;

void main()
{
	gl_Position = vec4(positioin, 1.0);
}
