#version 460
#extension GL_ARB_shading_language_include : require

#include "/shaders/LocationsAndBindings.h"
#include "/shaders/cameraDefs.glsl"
#include "/shaders/materialDefs.glsl"
#include "/shaders/lighting.glsl"

layout(binding = SSAO_TEX_BINDING) uniform sampler2D ssao;

in struct VertexOut
{
    vec2 uv0;
    vec2 uv1;
    vec3 normal;
    vec3 tangent;
    vec3 position;
    vec3 shadowCoord;
} fragment;

in flat int draw_id;

out vec4 color;

void sampleSSAO(inout PBR pbr)
{
    vec4 projectedPos = proj*view*vec4(fragment.position, 1.0);
    vec2 uv  = (projectedPos.xy/projectedPos.w)*0.5+0.5;

    float ssao = texture(ambientOcclusion, uv).r;
    pbr.occlusion *= ssao;
}

void main()
{
    PBR pbr;
    getMaterial(pbr, draw_id, fragment.uv0, fragment.position, fragment.normal, fragment.tangent, fragment.shadowCoord);

    sampleSSAO(pbr);

    color = Shading(pbr);
}
