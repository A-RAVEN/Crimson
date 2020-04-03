#version 450

#if VERTEX_SHADER
layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec4 inColor;

layout (binding = 0) uniform UITransform {
	vec2 scale;
	vec2 translate;
} transform;

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec4 outColor;

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

void main() 
{
	outUV = inUV;
	outColor = inColor;
	gl_Position = vec4(inPos * transform.scale + transform.translate, 0.0, 1.0);
}
#endif

#if FRAGMENT_SHADER
layout (binding = 1) uniform sampler2D fontSampler;

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

// TEXTURE_INPUTS(1);
// layout(push_constant) uniform PUSH_CONST_DATA
// {
// 	int texture_id;
// };

void main() 
{
	// if(texture_id >= 0)
	// {
	// 	outColor = texture(Textures[texture_id], inUV);
	// }
	// else
	// {
		outColor = inColor * texture(fontSampler, inUV).a;
	//}
}
#endif