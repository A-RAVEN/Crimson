#version 450

layout(location = 0) in vec4 in_normal;

layout(location = 0) out vec4 color;

layout(set = 0, binding = 1) uniform sampler2D test_img;

void main()
{
    //color = texture(test_img, vec2(0.5, 0.5)) * dot(normalize(in_normal.xyz), vec3(1.0, 1.0, 1.0));
    color = vec4(in_normal.xyz * 0.5 + 0.5, 1.0);
}