#version 450

layout(location = 0) out vec4 color;

layout(set = 0, binding = 1) uniform sampler2D test_img;

void main()
{
    color = texture(test_img, vec2(0.5, 0.5));
}