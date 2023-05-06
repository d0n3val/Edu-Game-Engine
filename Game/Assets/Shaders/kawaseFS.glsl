#version 460

#extension GL_ARB_shading_language_include : require

#include "/shaders/LocationsAndBindings.h"


layout(binding = KAWASE_INPUT_BINDING ) uniform sampler2D inputTex;
layout(location = KAWASE_INV_INPUT_SIZE_LOCATION) uniform vec2 invSizeLocation;
layout(location = KAWASE_INPUT_LOD ) uniform int inputLod;
layout(location = KAWASE_STEP ) uniform int step;

in vec2 uv;

out vec4 color;

void main()
{
    vec2 realStep = (float(step)+0.5)*invSizeLocation;

    vec3 sum = textureLod(inputTex, uv+vec2(-realStep.x, realStep.y), inputLod).rgb;
    sum += texture(inputTex, uv+vec2(realStep.x, realStep.y), inputLod).rgb;
    sum += texture(inputTex, uv+vec2(-realStep.x, -realStep.y), inputLod).rgb;
    sum += texture(inputTex, uv+vec2(realStep.x, -realStep.y), inputLod).rgb;

    color = vec4(sum/4.0, 1.0);
}