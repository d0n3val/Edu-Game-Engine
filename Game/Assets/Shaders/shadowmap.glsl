#version 460
#extension GL_ARB_shading_language_include : require
#extension GL_ARB_bindless_texture : require

layout(early_fragment_tests) in;

#define VARIANCE

#include "/shaders/LocationsAndBindings.h"
#include "/shaders/materialDefs.glsl"
#include "/shaders/vertexDefs.glsl"

in VertexOut fragment;
in flat int draw_id;

#ifdef VARIANCE
out vec4 colour;
#endif 

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

#ifdef VARIANCE
    colour = vec4(gl_FragCoord.z, gl_FragCoord.z*gl_FragCoord.z, 0.0, 0.0);
#endif 
}