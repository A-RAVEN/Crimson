#version 450

layout (binding = 1) uniform sampler2D fontSampler;

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

TEXTURE_INPUTS(1);


layout(push_constant) uniform PUSH_CONST_DATA
{
	int texture_id;
};

void main() 
{
	if(texture_id >= 0)
	{
		outColor = texture(Textures[texture_id], inUV);
	}
	else
	{
		outColor = inColor * texture(fontSampler, inUV).a;
	}
}