#version 450

layout(set = 0, binding = 0) uniform MATRIX
{
	mat4 matrix;
};

layout(location = 0) in vec3 positioin;

void main()
{
	gl_Position = matrix * vec4(positioin, 1.0);
}
