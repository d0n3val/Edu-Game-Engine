#version 460

#extension GL_ARB_shading_language_include : require

#include "/shaders/LocationsAndBindings.h"

layout(binding = DUALKAWASE_INPUT_BINDING ) uniform sampler2D inputTex;

in vec2 uv;

out vec4 color;

void main()
{
    vec2 texSize = vec2(textureSize(inputTex, 0));

    vec2 halfPixel = 0.5/texSize;

    vec4 sum = texture(inputTex, uv + vec2(-halfPixel.x * 2.0, 0.0));
    sum += texture(inputTex, uv + vec2(-halfPixel.x, halfPixel.y)) * 2.0;
    sum += texture(inputTex, uv + vec2(0.0, halfPixel.y * 2.0));
    sum += texture(inputTex, uv + vec2(halfPixel.x, halfPixel.y)) * 2.0;
    sum += texture(inputTex, uv + vec2(halfPixel.x * 2.0, 0.0));
    sum += texture(inputTex, uv + vec2(halfPixel.x, -halfPixel.y)) * 2.0;
    sum += texture(inputTex, uv + vec2(0.0, -halfPixel.y * 2.0));
    sum += texture(inputTex, uv + vec2(-halfPixel.x, -halfPixel.y)) * 2.0;

    color = sum / 12.0;
}

