#version 460
#extension GL_ARB_shading_language_include : require
#extension GL_ARB_bindless_texture : require

#include "/shaders/LocationsAndBindings.h"
#include "/shaders/cameraDefs.glsl"
#include "/shaders/materialDefs.glsl"
#include "/shaders/lighting.glsl"
#include "/shaders/vertexDefs.glsl"
#include "/shaders/shadows.glsl"

//#define DISABLE_SSAO

#ifndef DISABLE_SSAO
layout(binding = SSAO_TEX_BINDING) uniform sampler2D ssao;
#endif

in VertexOut fragment;
in flat int draw_id;

out vec4 color;

void sampleSSAO(inout PBR pbr)
{
#ifndef DISABLE_SSAO
    vec4 projectedPos = proj*view*vec4(fragment.geom.position, 1.0);
    vec2 uv  = (projectedPos.xy/projectedPos.w)*0.5+0.5;

    pbr.occlusion *= texture(ssao, uv).r;
#else
    pbr.occlusion = 1.0;
#endif 
}

void sampleShadow(inout PBR pbr)
{
#ifndef DISABLE_SHADOW
    pbr.shadow = computeShadow(pbr.position);
#else
    pbr.shadow = 1.0;
#endif 
}

void main()
{
    PBR pbr;
    getMaterial(pbr, draw_id, fragment.uv0, fragment.geom);

    sampleSSAO(pbr);
    sampleShadow(pbr);

    color = Shading(pbr);
}
