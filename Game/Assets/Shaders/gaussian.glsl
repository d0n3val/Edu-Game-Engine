#version 440
#extension GL_ARB_shading_language_include : require

#include "/shaders/common.glsl"
#include "/shaders/LocationsAndBindings.h"

out vec4 color;
in vec2 uv;

layout(binding = GAUSSIAN_BLUR_IMAGE_BINDING) uniform sampler2D sourceTexture;
layout(location = GAUSSIAN_BLUR_INVIMAGE_SIZE_LOCATION ) uniform vec2 invSize;
layout(location = GAUSSIAN_BLUR_SOURCE_LOD) uniform float sourceLod;

const int SAMPLE_COUNT = 3;

const float OFFSETS[3] = float[3](
    -1.3446745248463534,
    0.4466722983756714,
    2
);

const float WEIGHTS[3] = float[3](
    0.35564374091247164,
    0.5217749216739427,
    0.1225813374135857
);

#if HORIZONTAL
const vec2 blurDirection = vec2(1, 0);
#else
const vec2 blurDirection = vec2(0, 1);
#endif 

void main()
{
    vec4 result = vec4(0.0);
    for (int i = 0; i < SAMPLE_COUNT; ++i)
    {
        vec2 offset = blurDirection * OFFSETS[i] *invSize;
        float weight = WEIGHTS[i];
        result += textureLod(sourceTexture, uv + offset, sourceLod) * weight;
    }
    
    color = vec4(result);
}

