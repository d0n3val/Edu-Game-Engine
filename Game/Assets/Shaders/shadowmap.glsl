#version 460
#extension GL_ARB_shading_language_include : require

layout(early_fragment_tests) in;

#include "/shaders/LocationsAndBindings.h"
#include "/shaders/materialDefs.glsl"

in struct VertexOut
{
    vec2 uv0;
    vec2 uv1;
    vec3 normal;
    vec3 tangent;
    vec3 position;
} fragment;

in flat int draw_id;

void main()
{
    // Discard shadows from alpha fragments
    Material material = materials[draw_id]; 

    if((material.mapMask & DIFFUSE_MAP_FLAG) != 0)
    {
        if(sampleTexture(DIFFUSE_MAP_INDEX, fragment.uv0, draw_id).a < material.alphaTest)
        {
            discard;
        }
    }
}