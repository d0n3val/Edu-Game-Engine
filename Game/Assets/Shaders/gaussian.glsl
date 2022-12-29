#version 440
#extension GL_ARB_shading_language_include : require

#include "/shaders/common.glsl"
#include "/shaders/LocationsAndBindings.h"

out vec4 color;
in vec2 uv;

layout(binding = GAUSSIAN_BLUR_IMAGE_BINDING) uniform sampler2D image;

float weight[3] = float[] (0.38774,	0.24477, 0.06136);

void main()
{
    vec2 tex_offset = 1.0 / textureSize(image, 0); 
    vec3 result = texture(image, uv).rgb * weight[0]; // current fragment's contribution

#if HORIZONTAL
    for(int i = 1; i < 3; ++i)
    {
        result += texture(image, uv + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        result += texture(image, uv - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
    }
#else
    for(int i = 1; i < 3; ++i)
    {
        result += texture(image, uv + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        result += texture(image, uv - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
    }
#endif

    color = vec4(result, 1.0);
}

