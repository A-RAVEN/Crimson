#version 450


#include <camera.gh>
#include <encoding.gh>


layout(location = 0) in vec2 inFillRectPos;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D src_img;
layout(binding = 1) uniform sampler2D src_depth;
layout(binding = 2) uniform sampler2D src_normal;
layout(binding = 3) uniform sampler2D acc_img;
layout(binding = 4) uniform CAMERA
{
    SCameraData CameraData;
};

#define PI 3.1415926

void main()
{
    float window_size = 5;
    float gaussian_variance = window_size * 0.5 * 1.5;
    float gaussian_depth_variance = 0.001;
    float half_size = floor(window_size * 0.5);
	vec2 this_uv = inFillRectPos * 0.5 + 0.5;
    vec2 texture_delta = 1.0 / vec2(textureSize(src_img, 0));

    vec3 this_normal = normalize(texture(src_normal, this_uv).xyz * 2.0 - 1.0);
    float this_depth = texture(src_depth, this_uv).x;

    float gaussian_depth_variance_x2 = gaussian_depth_variance * 2;
    float gaussian_variance2 = gaussian_variance * gaussian_variance;
    float gaussian_variance2_x2 = gaussian_variance2 * 2;
    vec4 final_val = vec4(0.0);
    float weight = 0.0;

    float result = 0;
    for(float i = -half_size; i <= half_size; ++i)
    {
        for(float j = -half_size; j <= half_size; ++j)
        {
            vec2 sample_uv = this_uv + texture_delta * vec2(i, j);

            vec4 sample_val = texture(src_img, sample_uv);

            vec3 sample_normal = normalize(texture(src_normal, sample_uv).xyz);
            float sample_depth = texture(src_depth, sample_uv).x;
            float depth_variance = sample_depth - this_depth;

            float spatial_weight = 1.0 / (PI * gaussian_variance2_x2) * exp(-((i * i + j * j) / gaussian_variance2_x2));
            float range_weight = max(0.0001, dot(this_normal, sample_normal)) * 1.0 / sqrt(PI * gaussian_depth_variance_x2) * exp(-(depth_variance * depth_variance / gaussian_depth_variance_x2));

            final_val += spatial_weight * range_weight * sample_val;
            weight += spatial_weight * range_weight;
        }
    }
    if(weight <= 0.0)
    {
        //outColor = vec4(1.0);
        result = 1.0;
    }
    else
    {
	    //outColor = final_val / weight;
        result = final_val.x / weight;
    }

    vec4 reproj_pos = vec4(ProjectionInverse(CameraData.mInverseMat, vec3(inFillRectPos, this_depth)), 1.0);
    reproj_pos = CameraData.mInverseViewMat * reproj_pos;
    reproj_pos = CameraData.mLastInverseMat * reproj_pos;
    reproj_pos /= reproj_pos.w;

    vec2 absreproj = abs(reproj_pos.xy);
    if(absreproj.x < 0.99 && absreproj.y < 0.99)
    {
        vec2 last_uv = reproj_pos.xy * 0.5 + 0.5;

        vec2 sampled_value = texture(acc_img, last_uv).xw;
        sampled_value.y *= 10;
        sampled_value.x = (sampled_value.x * sampled_value.y + result) / (sampled_value.y + 1);
        sampled_value.y = min(6, sampled_value.y + 1) * 0.1;

        outColor = vec4(sampled_value.x, sampled_value.x, sampled_value.x, sampled_value.y);
    }
    else
    {
        outColor = vec4(result, result, result, 0.1);
    }
}