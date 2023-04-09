#version 440
#extension GL_ARB_shading_language_include : require

#include "/shaders/common.glsl"
#include "/shaders/LocationsAndBindings.h"

out vec4 color;
in vec2 uv;

layout(binding = GAUSSIAN_BLUR_IMAGE_BINDING) uniform sampler2D sourceTexture;
layout(location = GAUSSIAN_BLUR_INVIMAGE_SIZE_LOCATION ) uniform vec2 invSize;

const int SAMPLE_COUNT = 5;

const float OFFSETS[5] = float[5](
    -3.365259304013324,
    -1.4410698177487775,
    0.4802756349569519,
    2.402584067538288,
    4
);

const float WEIGHTS[5] = float[5](
    0.13222689205342525,
    0.2823767989937012,
    0.3286215336308134,
    0.20847767275878126,
    0.04829710256327899
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
        result += texture(sourceTexture, uv + offset) * weight;
    }
    
    color = vec4(result.rgb, 1.0);
}

